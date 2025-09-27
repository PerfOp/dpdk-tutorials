#include "debug_utils.h"

#include <stdint.h>
#ifdef _WIN32
#include <intrin.h>
#pragma intrinsic(__rdtsc)
#else
#include <x86intrin.h>
#endif

#include <sys/timerfd.h>
#include <unistd.h>

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>

volatile sig_atomic_t exit_indicator = 0;
void terminate(int signal) { exit_indicator = 1; }

uint64_t get_cycles() {
#ifdef _WIN32
    return __rdtsc();
#else
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
#endif
}

void dump_mem_hex(const void* addr, size_t len) {
    printf_error("Address: %p\n", addr);

    size_t print_len = len < 16 ? len : 16;
    const uint8_t* p = static_cast<const uint8_t*>(addr);

    printf_error("%zu bytes hex:\n", print_len);
    for (size_t i = 0; i < print_len; ++i) {
        printf_error("%02X ", p[i]);
    }
    printf_error("\n");
}

void Stats::Init(){
    lastCount=0;
    totalCount=0;
    statisticTimer.reset();
}
void Stats::Ticks(){
    uint64_t period=statisticTimer.nanoSeconds();
    uint64_t doneCount = totalCount - lastCount;
    lastCount=totalCount;
    statisticTimer.reset();
    spdlog::info("Stats: iops {:.2f} kpps", (double)(doneCount)/(double)(period/1000000));
}


// Ethernet header
struct ether_hdr {
    uint8_t dst_addr[6];
    uint8_t src_addr[6];
    uint16_t ether_type;
} __attribute__((packed));

// IPv4 header
struct ipv4_hdr {
    uint8_t version_ihl;
    uint8_t type_of_service;
    uint16_t total_length;
    uint16_t packet_id;
    uint16_t fragment_offset;
    uint8_t time_to_live;
    uint8_t next_proto_id;
    uint16_t hdr_checksum;
    uint32_t src_addr;
    uint32_t dst_addr;
} __attribute__((packed));

// UDP header
struct udp_hdr {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t dgram_len;
    uint16_t dgram_cksum;
} __attribute__((packed));

// Pseudo header for UDP checksum
struct pseudo_hdr {
    uint32_t src_addr;
    uint32_t dst_addr;
    uint8_t zero;
    uint8_t proto;
    uint16_t udp_len;
} __attribute__((packed));

// Calculate checksum
uint16_t calc_checksum(const uint8_t* data, size_t len) {
    uint32_t sum = 0;
    for (size_t i = 0; i < len; i += 2) {
        uint16_t word = data[i] << 8;
        if (i + 1 < len) word |= data[i + 1];
        sum += word;
    }
    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    return ~sum;
}

// Print MAC address
void print_mac(const uint8_t* mac) {
    for (int i = 0; i < 6; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(mac[i]);
        if (i < 5) std::cout << ":";
    }
}

int verify_mtu(uint8_t* packet) {
    // Simulate a UDP MTU packet (Ethernet + IPv4 + UDP)
    // uint8_t packet[1500];  // Assume max MTU size
    // memset(packet, 0, sizeof(packet));

    // Assume packet is filled with real data
    ether_hdr* eth = reinterpret_cast<ether_hdr*>(packet);
    ipv4_hdr* ip = reinterpret_cast<ipv4_hdr*>(packet + sizeof(ether_hdr));
    udp_hdr* udp = reinterpret_cast<udp_hdr*>(packet + sizeof(ether_hdr) + sizeof(ipv4_hdr));
    uint8_t* udp_payload = reinterpret_cast<uint8_t*>(udp + 1);

    // Check if Ethernet type is IPv4
    if (ntohs(eth->ether_type) != 0x0800) {
        std::cerr << "Not an IPv4 packet." << std::endl;
        return -1;
    }

    // Check if IP protocol is UDP
    if (ip->next_proto_id != 17) {
        std::cerr << "Not a UDP packet." << std::endl;
        return -1;
    }

    // Print MAC addresses
    std::cout << "Source MAC: ";
    print_mac(eth->src_addr);
    std::cout << "\nDestination MAC: ";
    print_mac(eth->dst_addr);
    std::cout << std::endl;

    // Print IP addresses
    struct in_addr src_ip, dst_ip;
    src_ip.s_addr = ip->src_addr;
    dst_ip.s_addr = ip->dst_addr;
    std::cout << "Source IP: " << inet_ntoa(src_ip) << std::endl;
    std::cout << "Destination IP: " << inet_ntoa(dst_ip) << std::endl;

    // Print UDP ports
    std::cout << "Source Port: " << ntohs(udp->src_port) << std::endl;
    std::cout << "Destination Port: " << ntohs(udp->dst_port) << std::endl;
    std::cout << "Payload length:" <<ntohs(udp->dgram_len)<<std::endl;

    // Check if checksum is required
    if (udp->dgram_cksum == 0) {
        std::cout << "UDP checksum is 0, no validation required." << std::endl;
    } else {
        // Build pseudo header
        pseudo_hdr ph;
        ph.src_addr = ip->src_addr;
        ph.dst_addr = ip->dst_addr;
        ph.zero = 0;
        ph.proto = 17;
        ph.udp_len = udp->dgram_len;

        // Combine pseudo header + UDP header + payload
        size_t udp_len = ntohs(udp->dgram_len);
        size_t total_len = sizeof(pseudo_hdr) + udp_len;
        uint8_t* buf = new uint8_t[total_len];
        memcpy(buf, &ph, sizeof(pseudo_hdr));
        memcpy(buf + sizeof(pseudo_hdr), udp, udp_len);

        uint16_t checksum = calc_checksum(buf, total_len);
        delete[] buf;

        std::cout << "Calculated UDP checksum: 0x" << std::hex << checksum << std::endl;
        if (checksum == 0) {
            std::cout << "UDP checksum is valid ✅" << std::endl;
        } else {
            std::cout << "UDP checksum is invalid ❌" << std::endl;
        }
    }

    return 0;
}

std::string get_current_data_time() {
    // Example of the very popular RFC 3339 format UTC time
    std::time_t time = std::time({});
    char timeString[std::size("yyyy-mm-ddThh:mm:ssZ")];
    std::strftime(std::data(timeString), std::size(timeString), "%FT%TZ",
                  std::gmtime(&time));
    return timeString;
}

std::atomic<int> counter(0);

void timerThread(void *pstats) {
    int tfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (tfd == -1) {
        return;
    }

    itimerspec new_value{};
    new_value.it_interval.tv_sec = 1;  // Trigger interval: 1 second
    new_value.it_value.tv_sec = 1;     // For the next trigger.
    timerfd_settime(tfd, 0, &new_value, nullptr);

    while (!exit_indicator) {
        uint64_t expirations;
        read(tfd, &expirations, sizeof(expirations));  // Block until to the expiraton
        counter += expirations;
        if (pstats) {
            Stats *ps = static_cast<Stats *>(pstats);
            ps->Ticks();
        }
    }

    close(tfd);
}
