// MIT License
//
// Copyright (c) 2024 Muhammad Awais Khalid
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <spdlog/fmt/bin_to_hex.h>
#include <spdlog/spdlog.h>

#include <iomanip>

#include "args.h"
#include "config.h"
#include "mem_worker.h"
#include "nic_worker.h"
#include "primary_worker.h"

struct PacketTransmissionThreadParams {
    uint16_t port_id = std::numeric_limits<decltype(port_id)>::max();
    uint16_t queue_id = std::numeric_limits<decltype(queue_id)>::max();
    uint16_t packets_per_second =
        std::numeric_limits<decltype(packets_per_second)>::min();
};

int transmit_packets_from_interface(void* param) {
    if (!param) {
        spdlog::error(
            "Unable to start packet transmission routine with null "
            "parameters.");
        return -1;
    }

    rte_mempool* mempool = rte_mempool_lookup(KNicPoolName.c_str());
    if (!mempool) {
        spdlog::error("Unable to lookup mempool: {}", KNicPoolName);
        return -1;
    }

    PacketTransmissionThreadParams* packetTransmissionThreadParams =
        reinterpret_cast<PacketTransmissionThreadParams*>(param);
    const uint16_t port_id = packetTransmissionThreadParams->port_id;
    const uint16_t queue_id = packetTransmissionThreadParams->queue_id;
    const uint64_t packet_len = sizeof(rte_ether_hdr) + sizeof(rte_ipv4_hdr) +
                                sizeof(rte_udp_hdr) + 1172;
    const uint64_t packets_per_second =
        packetTransmissionThreadParams->packets_per_second;
    const uint64_t packet_tx_burst_size = 16;
    const uint64_t interburst_time_ns =
        (1 * 1000000000) / (packets_per_second / packet_tx_burst_size);

    unsigned lcore_id = rte_lcore_id();
    cpu_set_t cpuset;
    pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    for (int i = 0; i < CPU_SETSIZE; i++) {
        if (CPU_ISSET(i, &cpuset)) {
            spdlog::warn(
                "Starting packet transmission n logical core: {}, Bound to CPU "
                "{} Portid {}",
                lcore_id, i, port_id);
        }
    }
    spdlog::warn("Benchmarking queue id: {} pps:{}", queue_id, packets_per_second);

    rte_mbuf* packets[packet_tx_burst_size];
    timespec ts{0};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t t0 = ts.tv_sec * 1000000000 + ts.tv_nsec;
    uint64_t t1 = t0 + interburst_time_ns;
    uint64_t tx_count = 0;

    while (!exit_indicator.load(std::memory_order_relaxed)) {
        /*
        while (t0 < t1) {
            clock_gettime(CLOCK_MONOTONIC, &ts);
            t0 = ts.tv_sec * 1000000000 + ts.tv_nsec;
        }
        t1 += interburst_time_ns;
        */

        /*packet = rte_pktmbuf_alloc(mempool);
        if (!packet) {
            std::cerr << "Unable to get memory buffer from mempool. " <<
        std::endl; using namespace std::literals;
            std::this_thread::sleep_for(50ms);
            continue;
        }*/

        if (rte_pktmbuf_alloc_bulk(mempool, packets, packet_tx_burst_size)) {
            using namespace std::literals;
            spdlog::error(
                "Unable to allocate the memory buffer in bulk from mempool. ");
            std::this_thread::sleep_for(50ms);
            continue;
        }

        // Setting the total packet size in our memory buffer.
        // Total packet size = Ethernet header size + IPv4 header size + UDP
        // header size + Payload size.

        for (uint16_t i = 0; i < packet_tx_burst_size; ++i) {
            packets[i]->data_len = packets[i]->pkt_len = packet_len;
            packets[i]->ol_flags = RTE_MBUF_F_TX_IP_CKSUM | RTE_MBUF_F_TX_IPV4 |
                                   RTE_MBUF_F_TX_UDP_CKSUM;
            packets[i]->l2_len = sizeof(rte_ether_hdr);
            packets[i]->l3_len = sizeof(rte_ipv4_hdr);
        }

        // Now our packet(s) are finally prepared. We will now send them using
        // the DPDK API. The DPDK API `rte_eth_tx_burst` will automatically
        // release the memory buffer(s) after tranmission is successful.
        tx_count = 0;
        do {
            tx_count += rte_eth_tx_burst(port_id, queue_id, &packets[tx_count],
                                         packet_tx_burst_size - tx_count);
        } while (tx_count < packet_tx_burst_size);
    }

    delete packetTransmissionThreadParams;
    return 0;
}

void init_process(BenchParam& benchParam) {
    // Detecting the logical cores (CPUs) ids passed to this DPDK application.
    uint16_t i = 0;
    std::vector<uint16_t> logicalCores;
    std::string corelist = "";
    RTE_LCORE_FOREACH(i) {
        logicalCores.push_back(i);
        corelist = corelist + " " + std::to_string(i);
    }
    spdlog::warn("Core list:{}", corelist);

    // We must have atleast one logical cores passed as an argument to this DPDK
    // application.
    if (logicalCores.size() != 1) {
        spdlog::error(
            "EAL:One logical core is required to run this DPDK application.");
        rte_eal_cleanup();
        exit(1);
    }
    // Find the process type of current process. primary/secondary.
    const rte_proc_type_t proc_type = rte_eal_process_type();

    if (proc_type == RTE_PROC_PRIMARY) {
        PrimaryProcess primaryProcess;
        primaryProcess.InitPrimaryResource(benchParam);

        // Start packet generation routine.
        primaryProcess.MainLoop();
        using namespace std::literals;
        std::this_thread::sleep_for(500ms);
    } else if (proc_type == RTE_PROC_SECONDARY) {
        /*
        MemProcess memProcess;
        memProcess.InitMemResource();

        // Start receiving and processing the packets.
        memProcess.MainLoop();
        */
        NicProcess nicProcess;
        nicProcess.InitNicResource();

        // Start receiving and processing the packets.
        nicProcess.MainLoop();
    }
}

int cross_core_call(int argc, char** argv, BenchParam& benchParam) {
    // 1)
    // Initializing the DPDK EAL (Environment Abstraction Layer). This is the
    // first step of a DPDK program before we call any further DPDK API. The
    // arguments passed to this programs are passed to rte_eal_init() DPDK API.
    // A user must pass DPDK EAL arguments before the application arguments. The
    // DPDK EAL arguments and application arguments must be separated by '--'.
    // For example: ./<dpdk_application> --lcores=0,1 -n 4 -- -s 1 -t 2. `--`
    // will tell the rte_eal_init() that all the DPDK EAL arguments are present
    // before this. In the above example the DPDK EAL arguments are --lcores and
    // -n. The user arguments are -s and -t. DPDK EAL argument `--lcores=0,1`
    // means that there are two logical cores (CPUs) assigned to this DPDK
    // application. The first logical core is 0 and second is 1. DPDK
    // application will launch total_logical_cores worker threads in the
    // application (including main) So in the above example, the DPDK
    // application has two logical cores (0,1). The main function will run on
    // first logical core (0) and an additional worker thread will be launched
    // on the next logical core (1). A DPDK application sets the affinity of
    // execution threads to specific logical cores to achieve performance.
    // DPDK EAL argument `-n 4` means that this DPDK application uses 4 memory
    // channels. The details are DPDK EAL arguments is present at:
    // https://doc.dpdk.org/guides/linux_gsg/linux_eal_parameters.html

    // rte_eal_init() DPDK API will return the number of DPDK EAL arguments
    // processed. So we will subtract the number of DPDK EAL arguments from the
    // total arguments and point argv to the first user argument. For example:
    // ./<dpdk_application> --lcores=0 -n 4 -- -s 1 -t 2 rte_eal_init() will
    // return 4. The total arguments passed to this program is 9. So after
    // subtracting the actual user arguments is (9 - 4 = 5). Setting `argv` to
    // point to the start of user argument which is `--`

    init_process(benchParam);

    rte_eal_cleanup();

    return 0;
}

int direct_nic_call(int argc, char** argv, BenchParam& benchParam) {
    // Setting up signals to catch TERM and INT signal.

    int32_t return_val = 0;
    spdlog::info("Starting DPDK program SP... ");

    // Initializing the DPDK EAL (Environment Abstraction Layer). This is the
    // first step of a DPDK program before we call any further DPDK API. The
    // arguments passed to this programs are passed to rte_eal_init() DPDK API.
    // A user must pass DPDK EAL arguments before the application arguments. The
    // DPDK EAL arguments and application arguments must be separated by '--'.
    // For example: ./<dpdk_application> --lcores=0 -n 4 -- -s 1 -t 2. `--` will
    // tell the rte_eal_init() that all the DPDK EAL arguments are present
    // before this. In the above example the DPDK EAL arguments are --lcores and
    // -n. The user arguments are -s and -t. DPDK EAL argument `--lcores=0`
    // means that this DPDK application will use core 0 to run the main function
    // (main thread). A DPDK application sets the affinity of execution threads
    // to specific logical cores to achieve performance. DPDK EAL argument `-n
    // 4` means that this DPDK application uses 4 memory channels. The details
    // are DPDK EAL arguments is present at:
    // https://doc.dpdk.org/guides/linux_gsg/linux_eal_parameters.html
    // return_val = rte_eal_init(argc, argv);
    // if (return_val < 0)
    // {
    // std::cerr << "Unable to initialize DPDK EAL (Environment Abstraction
    // Layer). Error code: " << rte_errno << std::endl; exit(1);
    // }

    // rte_eal_init() DPDK API will return the number of DPDK EAL arguments
    // processed. So we will subtract the number of DPDK EAL arguments from the
    // total arguments and point argv to the first user argument. For example:
    // ./<dpdk_application> --lcores=0 -n 4 -- -s 1 -t 2 rte_eal_init() will
    // return 4. The total arguments passed to this program is 9. So after
    // subtracting the actual user arguments is (9 - 4 = 5). Setting `argv` to
    // point to the start of user argument which is `--`

    // Detecting the logical cores (CPUs) ids passed to this DPDK application.
    uint16_t i = 0;
    std::vector<uint16_t> logicalCores;
    std::cout << "Logical cores ids (CPU ids): ";
    RTE_LCORE_FOREACH(i) {
        logicalCores.push_back(i);
        std::cout << i << " ";
    }
    std::cout << std::endl;

    // We must have atleast two logical cores passed as an argument to this DPDK
    // application. The first logical core will get and print the nic
    // statistics. The second logical core will execute the packet transmission
    // routine.
    if (logicalCores.size() != 2) {
        std::cerr
            << "Two logical cores are required to run this DPDK application. "
            << std::endl;
        rte_eal_cleanup();
        exit(1);
    }

    // Creating memory pool which contains the memory buffers. A memory buffer
    // is the buffer where DPDK driver will write an incoming packet. Below
    // memory pool has name "mempool_1" and has 65535 available memory buffer. A
    // single memory buffer has a size of RTE_MBUF_DEFAULT_BUF_SIZE (2048Bytes +
    // 128Bytes).
    rte_mempool* memory_pool =
        rte_pktmbuf_pool_create(KNicPoolName.c_str(), MEMORY_POOL_SIZE, 512, 0,
                                RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

    NicInfo nicInfo;
    nicInfo.ValidatePci(benchParam.port_pci, memory_pool, 0, 1);
    memcpy(benchParam.src_mac, nicInfo.GetRawMac(),
           sizeof(nicInfo.GetRawMac()));

    // Prepare memory pool.
    rte_mempool* tx_mempool = rte_mempool_lookup(KNicPoolName.c_str());
    if (!tx_mempool) {
        spdlog::error("Can not get valid mempool against {}", KNicPoolName);
        nicInfo.StopNic();
        rte_eal_cleanup();
        exit(1);
    }
    if (!preset_align_memory_pool(tx_mempool, 1024, benchParam)) {
        nicInfo.StopNic();
        rte_eal_cleanup();
        exit(1);
    }

    // Now initiating packet transmission routine on the second logical core id.
    PacketTransmissionThreadParams* packetTransmissionThreadParams =
        new PacketTransmissionThreadParams;
    packetTransmissionThreadParams->port_id = nicInfo.port_id;
    packetTransmissionThreadParams->queue_id = 0;
    packetTransmissionThreadParams->packets_per_second = 300000;
    if ((return_val = rte_eal_remote_launch(
             transmit_packets_from_interface,
             reinterpret_cast<void*>(packetTransmissionThreadParams),
             logicalCores[1])) != 0) {
        spdlog::error(
            "Unable to launch packet transmission routine on the logical core: "
            "{}. Return code: {}",
            logicalCores[1], return_val);
        nicInfo.StopNic();
        rte_eal_cleanup();
        exit(1);
    }

    using namespace std::literals;
    std::this_thread::sleep_for(1000ms);

    // Logical core 0 will get and print nic statistics.
    get_and_print_nic_statistics(nicInfo.port_id);

    // Now we will wait for all the lcores (except main lcore = 0) to finish
    // before we exit the application.
    for (uint16_t i = 1; i < logicalCores.size(); ++i) {
        spdlog::warn("Waiting for logical core {} to join.", logicalCores[i]);
        rte_eal_wait_lcore(logicalCores[i]);
    }

    spdlog::info("Exiting DPDK program ... ");
    rte_eal_cleanup();
    return 0;
}

int main(int argc, char** argv) {
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = terminate;
    sigaction(SIGTERM, &action, nullptr);
    sigaction(SIGINT, &action, nullptr);

    int32_t return_val = rte_eal_init(argc, argv);
    if (return_val < 0) {
        spdlog::error("EAL: Unable to initialize DPDK EAL. Error code: {}",
                      rte_errno);
        exit(1);
    }
    argc -= return_val;
    argv += return_val;

    BenchParam benchParam;
    parse_args(argc, argv, benchParam);
    // return cross_core_call(argc, argv, benchParam);
    return direct_nic_call(argc, argv, benchParam);
}
