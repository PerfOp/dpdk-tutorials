/**
 * Copyright (C) The software Authors. All rights reserved.
 * File Name: bypass/mem_worker.cpp
 * Author:
 * mail:
 * Created Time: Wed Sep 24 05:20:14 2025
 * Brief:
 */
#include "nic_worker.h"

#include <spdlog/fmt/bin_to_hex.h>

#include "config.h"

bool prepare_memory_pool(rte_mempool *mempool, const BenchParam &benchparam) {
    // rte_mempool *mempool = rte_mempool_lookup(KNicPoolName.c_str());
    if (!mempool) {
        spdlog::error("Invalid pool in prepare_memory_pool!");
        return false;
    }

    std::vector<rte_mbuf *> memory_buffers;
    memory_buffers.resize(KNicBufCount);

    uint32_t i = 0;
    for (; i < KNicBufCount; ++i) {
        rte_mbuf *buffer = rte_pktmbuf_alloc(mempool);
        memory_buffers[i] = buffer;
    }

    if (i != KNicBufCount) {
        spdlog::error("Not all the memory buffers are available in mempool: {}",
                      KNicPoolName);
        return false;
    }

    uint8_t temp = 0;

    for (uint32_t j = 0; j < memory_buffers.size(); ++j) {
        // Prepare the memory buffer.
        rte_mbuf *buf = memory_buffers[j];

        // We will get a pointer to the main memory area of our memory buffer
        // and write packet info.
        uint8_t *data = rte_pktmbuf_mtod(buf, uint8_t *);

        // Setting Ethernet header information (Source MAC, Destination MAC,
        // Ethernet type).
        rte_ether_hdr *const eth_hdr = reinterpret_cast<rte_ether_hdr *>(data);
        eth_hdr->ether_type = rte_cpu_to_be_16(RTE_ETHER_TYPE_IPV4);

        // const uint8_t src_mac_addr[6] = {0x08, 0x00, 0x27, 0x95, 0xBD, temp};
        // memcpy(eth_hdr->src_addr.addr_bytes, src_mac_addr,
        // sizeof(src_mac_addr));
        memcpy(eth_hdr->src_addr.addr_bytes, benchparam.src_mac,
               sizeof(benchparam.src_mac));
        // spdlog::info("packet header src
        // mac{}",spdlog::to_hex(eth_hdr->src_addr.addr_bytes,
        // eth_hdr->src_addr.addr_bytes + 6));

        // const uint8_t dst_mac_addr[6] = {0x08, 0x00, 0x27, 0x35, 0x14, temp};
        // memcpy(eth_hdr->dst_addr.addr_bytes, dst_mac_addr,
        // sizeof(dst_mac_addr));
        memcpy(eth_hdr->dst_addr.addr_bytes, benchparam.dst_mac,
               sizeof(benchparam.dst_mac));
        // spdlog::info("packet header dst
        // mac{}",spdlog::to_hex(eth_hdr->dst_addr.addr_bytes,
        // eth_hdr->dst_addr.addr_bytes + 6));

        // Setting IPv4 header information.
        rte_ipv4_hdr *const ipv4_hdr =
            reinterpret_cast<rte_ipv4_hdr *>(data + sizeof(rte_ether_hdr));
        ipv4_hdr->version = 4;  // Setting IP version as IPv4
        ipv4_hdr->ihl =
            5;  // Setting IP header length = 20 bytes = (5 * 4 Bytes)
        ipv4_hdr->type_of_service = 0;  // Setting DSCP = 0; ECN = 0;
        ipv4_hdr->total_length = rte_cpu_to_be_16(
            200);  // Setting total IPv4 packet length to 200 bytes. This
                   // includes the IPv4 header (20 bytes) as well.
        ipv4_hdr->packet_id =
            0;  // Setting identification = 0 as the packet is non-fragmented.
        ipv4_hdr->fragment_offset = 0x0040;  // Setting packet as non-fragmented
                                             // and fragment offset = 0.
        ipv4_hdr->time_to_live = 64;         // Setting Time to live = 64;
        ipv4_hdr->next_proto_id = 17;  // Setting the next protocol as UDP (17).

        // const uint8_t src_ip_addr[4] = {10, 10, 8, temp};
        // memcpy(&ipv4_hdr->src_addr, src_ip_addr,
        // sizeof(src_ip_addr));  // Setting source ip address = 1.2.3.4
        memcpy(
            &ipv4_hdr->src_addr, benchparam.src_ip,
            sizeof(benchparam.src_ip));  // Setting source ip address = 1.2.3.4

        const uint8_t dest_ip_addr[4] = {100, 10, 100, temp};
        // memcpy(
        // &ipv4_hdr->dst_addr, dest_ip_addr,
        // sizeof(dest_ip_addr));  // Setting destination ip address = 4.3.2.1
        memcpy(
            &ipv4_hdr->dst_addr, benchparam.dst_ip,
            sizeof(benchparam.dst_ip));  // Setting source ip address = 1.2.3.4

        ++temp;

        // Calculating and setting IPv4 checksum in IPv4 header.
        // ipv4_hdr->hdr_checksum = rte_ipv4_cksum(ipv4_hdr);      //
        ipv4_hdr->hdr_checksum = 0;

        // Setting UDP header information.
        rte_udp_hdr *const udp_hdr = reinterpret_cast<rte_udp_hdr *>(
            data + sizeof(rte_ether_hdr) + sizeof(rte_ipv4_hdr));
        udp_hdr->dst_port =
            rte_cpu_to_be_16(benchparam.dst_port);  // Setting destination port.
        // rte_cpu_to_be_16(5566 + temp);  // Setting destination port.
        udp_hdr->src_port =
            rte_cpu_to_be_16(benchparam.src_port);  // Setting source port.
        // rte_cpu_to_be_16(9988 + temp);           // Setting source port.
        udp_hdr->dgram_len = rte_cpu_to_be_16(180);  // Setting datagram length.
        // udp_hdr->dgram_cksum = rte_ipv4_phdr_cksum(
            // ipv4_hdr, 0);  // Setting checksum of ip psuedo header.
        udp_hdr->dgram_cksum = 0;

        // Setting data in the UDP payload
        uint8_t *payload = data + sizeof(rte_ether_hdr) + sizeof(rte_ipv4_hdr) +
                           sizeof(rte_udp_hdr);
        memset(payload, 0, 172);
        std::string sample_data =
            "This is a sample data generated by a DPDK application ..." +
            std::to_string(j);
        // const char sample_data[] = {
        // "This is a sample data generated by a DPDK application ..."};
        memcpy(payload, sample_data.c_str(), sizeof(sample_data.size()));

        if (j == 0) {
            verify_mtu(data);
        }

        // Return the memory buffer to memory pool.
        rte_pktmbuf_free(buf);
        buf = memory_buffers[j] = nullptr;
    }

    memory_buffers.clear();
    return true;
}

bool NicProcess::InitNicResource() {
    // Register a timestamp dynamic field.
    // m_attachDataQueue = new DynaQueue();
    // Primary process with look up for the ring buffer and receive the
    // packets generated by primary DPDK application. Lookup for the ring
    // buffer created by a primary application.
    // if (!m_attachDataQueue->attach_ring(ringname)) {

    // Get the shared port_id of the initialized nic from primary process
    m_pHandleZone = rte_memzone_lookup(KHandlerZone.c_str());
    if (m_pHandleZone != nullptr) {
        m_portId = *((uint16_t *)m_pHandleZone->addr);
        spdlog::warn("Get the nic handle id:{}", m_portId);
    } else {
        spdlog::error("Failed to lookup the handler zone:{}", KHandlerZone);
        exit(1);
    }

    // Attach the poo and ring created from the primary process
    if (!m_nicPool.attach_pool(KNicPoolName)) {
        exit(1);
    }
    if (!m_nicRing.attach_ring(KNicRingName)) {
        exit(1);
    }
    return InitResource();
}

void NicProcess::issue_request() {
    rte_mbuf *const packet = m_cmdPool.allocate_mbuf();
    if (!packet) {
        spdlog::error("Failed to allocate mbuf from cmd pool {}", KCmdPoolName);
        exit(1);
    } else {
        spdlog::info("Cmd buf created on {}.", KCmdRingName);
    }
    if (!m_cmdRing.produce_packets(packet, 1)) {
        spdlog::info("Cmd Ring full: {}", KCmdRingName);
        rte_pktmbuf_free(packet);
    }
}

int NicProcess::recv_loop() {
    uint8_t rx_count = 0;
    uint64_t total_rx_packets = 0;
    uint64_t lastTimestamp = 0;
    rte_mbuf *rx_packets[32];

    // We will print the logical core id (CPU id) on which this thread is
    // going to be executed. rte_lcore_id() function will return the current
    // logical core id (CPU id).
    spdlog::info("NicProcessor starting loop routine. Logical core id {}",
                 rte_lcore_id());

    issue_request();

    std::thread st(timerThread, &this->nicStats);
    // Now continuously monitor the ring buffer for any incoming packets.
    while (!exit_indicator) {
        // Check for any incoming packets in the ring buffer. We try to
        // dequeue max 32 packets at max at a time.
        rx_count = m_dataRing.consume_packets(rx_packets, 1);

        if (!rx_count) {
            // No packets are present in ring buffer. Check again.
            // using namespace std::literals;
            // std::this_thread::sleep_for(50us);
            continue;
        }
        //        spdlog::info("{} packets to display!", rx_count);

        // Packets received. Now we will process them.
        uint8_t done = 0;
        for (; done < rx_count; done++) {
            rte_mbuf *const packet = rx_packets[done];

            // Get the timestamp of the received memory buffer (packet).
            const uint64_t timestamp = *(RTE_MBUF_DYNFIELD(
                packet, m_attachDataQueue.get_offset(), uint64_t *));

            /*
            if (!(nicStats.totalCount % 10000)) {
                uint8_t *data = rte_pktmbuf_mtod(packet, uint8_t *);
                spdlog::info("packet header mac{}",
                             spdlog::to_hex(data, data + 12));
            }
            */
            /*
                        if (timestamp < lastTimestamp) {
                            std::cerr << get_current_data_time()
                                      << " The received timestamp is less than
               last " "time stamp. "
                                      << lastTimestamp << ":" << timestamp <<
               ":"
                                      << packet->data_len << ":" << packet <<
               std::endl;
                        }
                        lastTimestamp = timestamp;
            */
            rte_pktmbuf_free(packet);
        }
        nicStats.totalCount += done;
    }
    st.join();

    spdlog::info("Total packets received: {}", total_rx_packets);
    return 0;
}
