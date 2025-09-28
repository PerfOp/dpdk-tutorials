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

#include "primary_worker.h"
#include "mem_worker.h"
#include "nic_worker.h"
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <iomanip>
#include "args.h"
#include "config.h"

constexpr uint16_t NIC_STATISTICS_INTERVAL_MSEC = 1000;         // 1 seconds.

struct PacketTransmissionThreadParams {
    uint16_t port_id = std::numeric_limits<decltype(port_id)>::max();
    uint16_t queue_id = std::numeric_limits<decltype(queue_id)>::max();
    uint16_t packets_per_second = std::numeric_limits<decltype(packets_per_second)>::min();
};


int get_and_print_nic_statistics(const uint16_t port_id)
{
    int return_val = -1;
    std::chrono::time_point<std::chrono::system_clock> t1 = std::chrono::system_clock::now();
    rte_eth_stats stats = {0};
    uint64_t last_rx_bytes = 0;
    uint64_t last_tx_bytes = 0;
    uint64_t last_rx_packets = 0;
    uint64_t last_tx_packets = 0;

    std::cout << "Starting nic statistics routine on logical core: " << rte_lcore_id() << std::endl;

    while (!exit_indicator.load(std::memory_order_relaxed)) {
        std::chrono::time_point<std::chrono::system_clock> t2 = std::chrono::system_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

        if (diff.count() >= NIC_STATISTICS_INTERVAL_MSEC) {
            t1 = t2;
            std::cout << "\033[2J\033[1;1H";
            if ((return_val = rte_eth_stats_get(port_id, &stats) == 0)) {
                auto now = std::chrono::system_clock::now();
                auto in_time_t = std::chrono::system_clock::to_time_t(now);

                const double rx_packet_rate = (static_cast<double>(stats.ipackets - last_rx_packets) / (static_cast<double>(diff.count()) / 1000.0));
                last_rx_packets = stats.ipackets;
                const double tx_packet_rate = (static_cast<double>(stats.opackets - last_tx_packets) / (static_cast<double>(diff.count()) / 1000.0));
                last_tx_packets = stats.opackets;

                const double rx_data_rate = (static_cast<double>((stats.ibytes - last_rx_bytes) * 8) / (static_cast<double>(diff.count()) / 1000.0)) / (1024.0 * 1024.0);
                last_rx_bytes = stats.ibytes;
                const double tx_data_rate = (static_cast<double>((stats.obytes - last_tx_bytes) * 8) / (static_cast<double>(diff.count()) / 1000.0)) / (1024.0 * 1024.0);
                last_tx_bytes = stats.obytes;

                std::cout << std::endl;
                std::cout << "Ethernet Port: " << port_id << " Statistics" << std::endl;
                std::cout << "----------------------------------------------" << std::endl;
                std::cout << "Statistics time: " << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X") << std::endl;
                std::cout << "Receive  packets: " << stats.ipackets << std::endl;
                std::cout << "Transmit packets: " << stats.opackets << std::endl;
                std::cout << "Receive  bytes: " << stats.ibytes << std::endl;
                std::cout << "Transmit bytes: " << stats.obytes << std::endl;
                std::cout << "Receive  errors: " << stats.ierrors << std::endl;
                std::cout << "Transmit errors: " << stats.oerrors << std::endl;
                std::cout << "Rx rx_nombuf: " << stats.rx_nombuf << std::endl;
                std::cout << std::endl;
                std::cout << "Receive  data rate (mbps): " << rx_data_rate << std::endl;
                std::cout << "Transmit data rate (mbps): " << tx_data_rate << std::endl;
                std::cout << std::fixed << std::setprecision(1) << "Receive  packet rate (pps): " << rx_packet_rate << std::endl;
                std::cout << std::fixed << std::setprecision(1) << "Transmit packet rate (pps): " << tx_packet_rate << std::endl;
                std::cout << "----------------------------------------------" << std::endl;
                std::cout << std::endl;
            } else {
                std::cerr << "Unable to get ethernet device statistics. Port id: " << port_id << " Return value: " << return_val << std::endl;
            }
        }

        using namespace std::literals;
        std::this_thread::sleep_for(50ms);
    }

    return 0;
}

int transmit_packets_from_interface(void* param)
{
    if (!param) {
        std::cerr << "Unable to start packet transmission routine. Parameters are null. " << std::endl;
        return -1;
    }

    rte_mempool* mempool = rte_mempool_lookup(KNicPoolName.c_str());
    if (!mempool) {
        std::cerr << "Unable to lookup mempool: " << KNicPoolName << std::endl;
        return -1;
    }

    PacketTransmissionThreadParams *packetTransmissionThreadParams = reinterpret_cast<PacketTransmissionThreadParams *>(param);
    const uint16_t port_id = packetTransmissionThreadParams->port_id;
    const uint16_t queue_id = packetTransmissionThreadParams->queue_id;
    const uint64_t packet_len = sizeof(rte_ether_hdr) + sizeof(rte_ipv4_hdr) + sizeof(rte_udp_hdr) + 1172;
    const uint64_t packets_per_second = packetTransmissionThreadParams->packets_per_second;
    const uint64_t packet_tx_burst_size = 16;
    const uint64_t interburst_time_ns = (1 * 1000000000) / (packets_per_second / packet_tx_burst_size);

    std::cout << "Starting packet transmission routine on logical core: " << rte_lcore_id() << " Port id: " << port_id << " Queue id: " << queue_id
              << " Packets per second: " << packets_per_second << std::endl;

    rte_mbuf *packets[packet_tx_burst_size];
    timespec ts {0};
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
            std::cerr << "Unable to get memory buffer from mempool. " << std::endl;
            using namespace std::literals;
            std::this_thread::sleep_for(50ms);
            continue;
        }*/

        if (rte_pktmbuf_alloc_bulk(mempool, packets, packet_tx_burst_size)) {
            std::cerr << "Unable to allocate the memory buffer in bulk from mempool. " << std::endl;
            using namespace std::literals;
            std::this_thread::sleep_for(50ms);
            continue;
        }

        // Setting the total packet size in our memory buffer.
        // Total packet size = Ethernet header size + IPv4 header size + UDP header size + Payload size.

        for (uint16_t i = 0; i < packet_tx_burst_size; ++i) {
            packets[i]->data_len = packets[i]->pkt_len = packet_len;
            packets[i]->ol_flags = RTE_MBUF_F_TX_IP_CKSUM | RTE_MBUF_F_TX_IPV4 | RTE_MBUF_F_TX_UDP_CKSUM;
            packets[i]->l2_len = sizeof(rte_ether_hdr);
            packets[i]->l3_len = sizeof(rte_ipv4_hdr);
        }

        // Now our packet(s) are finally prepared. We will now send them using the DPDK API.
        // The DPDK API `rte_eth_tx_burst` will automatically release the memory buffer(s) after tranmission is successful.
        tx_count = 0;
        do {
            tx_count += rte_eth_tx_burst(port_id, queue_id, &packets[tx_count], packet_tx_burst_size - tx_count);
        } while (tx_count < packet_tx_burst_size);
    }

    delete packetTransmissionThreadParams;
    return 0;
}

/*
void application_usage() {
    std::cout << std::endl;
    std::cout << "Application usage:" << std::endl;
    std::cout << "------------------" << std::endl;
    std::cout << "sudo ./packet-generator -l <cores_ids> -n 4 --file-prefix=packet-gen -b <port_id_to_skip> -- --output-port <output_port_id> "
                 "--packets-per-second <packets_per_second>" << std::endl;
    std::cout << "Example: sudo ./packet-generator -l 4-5 -n 4 --file-prefix=packet-gen -b 0000:00:08.0 -- --output-port 0000:00:09.0 --packets-per-second 30000" << std::endl;
}
*/

void init_process(BenchParam& benchParam){
    // Detecting the logical cores (CPUs) ids passed to this DPDK application.
    uint16_t i = 0;
    std::vector<uint16_t> logicalCores;
    std::string corelist="";
    RTE_LCORE_FOREACH(i) {
        logicalCores.push_back(i);
        corelist=corelist+" "+std::to_string(i);
    }
    spdlog::warn("Core list:{}", corelist);

    // We must have atleast one logical cores passed as an argument to this DPDK
    // application.
    if (logicalCores.size() != 1) {
        spdlog::error("EAL:One logical core is required to run this DPDK application.");
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

int cross_core_call(int argc, char** argv, BenchParam& benchParam){
    // Setting up signals to catch TERM and INT signal.
    // struct sigaction action;
    // memset(&action, 0, sizeof(struct sigaction));
    // action.sa_handler = terminate;
    // sigaction(SIGTERM, &action, nullptr);
    // sigaction(SIGINT, &action, nullptr);

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

int direct_nic_call(int argc, char** argv, BenchParam& benchParam){
    // Setting up signals to catch TERM and INT signal.

    std::cout << "Starting DPDK program SP... " << std::endl;
    int32_t return_val=0;

    // Initializing the DPDK EAL (Environment Abstraction Layer). This is the first step of a DPDK program before we
    // call any further DPDK API.
    // The arguments passed to this programs are passed to rte_eal_init() DPDK API. A user must pass DPDK EAL arguments
    // before the application arguments. The DPDK EAL arguments and application arguments must be separated by '--'.
    // For example: ./<dpdk_application> --lcores=0 -n 4 -- -s 1 -t 2. `--` will tell the rte_eal_init() that all the DPDK
    // EAL arguments are present before this.
    // In the above example the DPDK EAL arguments are --lcores and -n. The user arguments are -s and -t.
    // DPDK EAL argument `--lcores=0` means that this DPDK application will use core 0 to run the main function (main thread).
    // A DPDK application sets the affinity of execution threads to specific logical cores to achieve performance.
    // DPDK EAL argument `-n 4` means that this DPDK application uses 4 memory channels.
    // The details are DPDK EAL arguments is present at: https://doc.dpdk.org/guides/linux_gsg/linux_eal_parameters.html
    // return_val = rte_eal_init(argc, argv);
    // if (return_val < 0)
    // {
        // std::cerr << "Unable to initialize DPDK EAL (Environment Abstraction Layer). Error code: " << rte_errno << std::endl;
        // exit(1);
    // }

    // rte_eal_init() DPDK API will return the number of DPDK EAL arguments processed. So we will subtract the number of DPDK EAL
    // arguments from the total arguments and point argv to the first user argument.
    // For example: ./<dpdk_application> --lcores=0 -n 4 -- -s 1 -t 2
    // rte_eal_init() will return 4. The total arguments passed to this program is 9. So after subtracting the actual user arguments
    // is (9 - 4 = 5). Setting `argv` to point to the start of user argument which is `--`

    NicInfo nicInfo;
    nicInfo.SetPci(benchParam.port_pci);
/*
    std::string output_port=benchParam.port_pci;
    uint32_t packets_per_second {30000};

    uint16_t port_ids[RTE_MAX_ETHPORTS] = {0};
    int16_t id = 0;
    int16_t total_port_count = 0;

    // Detecting the available ports (ethernet interfaces) in the system.
    RTE_ETH_FOREACH_DEV(id) {
        port_ids[total_port_count] = id;
        total_port_count++;
        if (total_port_count >= RTE_MAX_ETHPORTS)
        {
            std::cerr << "Total number of detected ports exceeds RTE_MAX_ETHPORTS. " << std::endl;
            rte_eal_cleanup();
            exit(1);
        }
    }

    if (total_port_count == 0) {
        std::cerr << "No ports detected in the system. " << std::endl;
        rte_eal_cleanup();
        exit(1);
    }

    std::cout << "Total ports detected: " << total_port_count << std::endl;

    uint16_t output_port_id = std::numeric_limits<decltype(output_port_id)>::max();
    if (rte_eth_dev_get_port_by_name(output_port.c_str(), &output_port_id)) {
        std::cerr << "Unable to get port id against port: " << output_port << std::endl;
    }

    // Check about the RX/TX offloading support of current ethernet device.
    // A ethernet device from different vendors (Intel, Nvidia, Broadcom etc.) supports different Rx/Tx offloading capabilities.
    // So we first check which Rx/Tx offloading capabilities are supported by our ether device.
    rte_eth_dev_info devInfo;
    if (!check_device_offloading_support(output_port_id, devInfo)) {
        rte_eal_cleanup();
        exit(1);
    }

    struct rte_ether_addr mac;
    rte_eth_macaddr_get(output_port_id, &mac);
    memcpy(benchParam.src_mac, mac.addr_bytes, sizeof(mac.addr_bytes));
*/
    memcpy(benchParam.src_mac, nicInfo.GetRawMac(), sizeof(nicInfo.GetRawMac()));
    // Detecting the logical cores (CPUs) ids passed to this DPDK application.
    uint16_t i = 0;
    std::vector<uint16_t> logicalCores;
    std::cout << "Logical cores ids (CPU ids): ";
    RTE_LCORE_FOREACH(i) {
        logicalCores.push_back(i);
        std::cout << i << " ";
    }
    std::cout << std::endl;

    // We must have atleast two logical cores passed as an argument to this DPDK application. The first logical core will get and print the nic statistics.
    // The second logical core will execute the packet transmission routine.
    if (logicalCores.size() != 2)
    {
        std::cerr << "Two logical cores are required to run this DPDK application. " << std::endl;
        rte_eal_cleanup();
        exit(1);
    }

    // Creating memory pool which contains the memory buffers. A memory buffer is the buffer where DPDK driver will write an
    // incoming packet. Below memory pool has name "mempool_1" and has 65535 available memory buffer. A single memory buffer
    // has a size of RTE_MBUF_DEFAULT_BUF_SIZE (2048Bytes + 128Bytes).
    rte_mempool *memory_pool = rte_pktmbuf_pool_create(KNicPoolName.c_str(), MEMORY_POOL_SIZE, 512, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

    // Configuring the port (ethernet interface). An ethernet interface can have multiple receive queues and transmit queues.
    // Currently we are setting up one transmit queue and no receive queue as we are not receiving packets in this tutorial.
    const uint16_t rx_queues = 0;
    const uint16_t tx_queues = 1;

    rte_eth_conf portConf = {
        .rxmode = {
            .mq_mode = RTE_ETH_MQ_RX_NONE
        },
        .txmode = {
            .mq_mode = RTE_ETH_MQ_TX_NONE,
            //.offloads = (devInfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_IPV4_CKSUM)
            .offloads = (nicInfo.devInfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_IPV4_CKSUM)
        }
    };

    // Configure the port (ethernet interface).
    if ((return_val = rte_eth_dev_configure(nicInfo.port_id, rx_queues, tx_queues, &portConf)) != 0) {
        std::cerr << "Unable to configure port. port Id: " << nicInfo.port_id << " Return code: "  << return_val << std::endl;
        rte_eal_cleanup();
        exit(1);
    }

    const int16_t portSocketId = rte_eth_dev_socket_id(nicInfo.port_id);
    const int16_t coreSocketId = rte_socket_id();

    // Configure the Rx queue(s) of the port.
    for (uint16_t i = 0; i < rx_queues; i++) {
        return_val = rte_eth_rx_queue_setup(nicInfo.port_id, i, 256, ((portSocketId >= 0) ? portSocketId : coreSocketId), nullptr, memory_pool);

        if (return_val < 0) {
            std::cerr << "Unable to setup RX queue " << i << " Port Id: " << nicInfo.port_id << "Return code: " << return_val << std::endl;
            rte_eal_cleanup();
            exit(1);
        }

        std::cout << "Port Id: " << nicInfo.port_id << " Rx Queue: " << i << " setup successful. Socket id: "
                  << ((portSocketId >= 0) ? portSocketId : coreSocketId) << std::endl;
    }

    // Configure the Tx queue(s) of the port.
    for (uint16_t i = 0; i < tx_queues; i++) {
        return_val = rte_eth_tx_queue_setup(nicInfo.port_id, i, 1024, ((portSocketId >= 0) ? portSocketId : coreSocketId), nullptr);

        if (return_val < 0) {
            std::cerr << "Unable to setup TX queue " << i << " Port Id: " << nicInfo.port_id << "Return code: " << return_val << std::endl;
            rte_eal_cleanup();
            exit(1);
        }

        std::cout << "Port Id: " << nicInfo.port_id << " Tx Queue: " << i << " setup successful. Port socket id: " << portSocketId
                  << " Core socket id: " << coreSocketId << std::endl;
    }

    // Enable promiscuous mode on the port. Not all the DPDK drivers provide the functionality to enable promiscuous mode. So we are going to
    // ignore the result if the API fails.
    return_val = rte_eth_promiscuous_enable(nicInfo.port_id);
    if (return_val < 0) {
        std::cout << "Warning: Unable to set the promiscuous mode for port Id: " << nicInfo.port_id << " Return code: " << return_val << " Ignoring ... " << std::endl;
    }

    // All the configuration is done. Finally starting the port (ethernet interface) so that we can start transmitting the packets.
    return_val = rte_eth_dev_start(nicInfo.port_id);
    if (return_val < 0) {
        std::cout << "Unable to start port Id: " << nicInfo.port_id << " Return code: " << return_val << std::endl;
        rte_eal_cleanup();
        exit(1);
    }

    std::cout << "Port configuration successful. Port Id: " << nicInfo.port_id << std::endl;

    // Prepare memory pool.
    if (!preset_memory_pool(1024, benchParam)) {
        rte_eth_dev_stop(nicInfo.port_id);
        rte_eth_dev_close(nicInfo.port_id);
        rte_eal_cleanup();
        exit(1);
    }

    // Now initiating packet transmission routine on the second logical core id.
    PacketTransmissionThreadParams *packetTransmissionThreadParams = new PacketTransmissionThreadParams;
    packetTransmissionThreadParams->port_id = nicInfo.port_id;
    packetTransmissionThreadParams->queue_id = 0;
    packetTransmissionThreadParams->packets_per_second = 300000;
    if ((return_val = rte_eal_remote_launch(transmit_packets_from_interface, reinterpret_cast<void *>(packetTransmissionThreadParams), logicalCores[1])) != 0) {
        std::cerr << "Unable to launch packet transmission routine on the logical core: %d. Return code: %d" << logicalCores[1] << return_val << std::endl;
        rte_eth_dev_stop(nicInfo.port_id);
        rte_eth_dev_close(nicInfo.port_id);
        rte_eal_cleanup();
        exit(1);
    }

    using namespace std::literals;
    std::this_thread::sleep_for(1000ms);

    // Logical core 0 will get and print nic statistics.
    get_and_print_nic_statistics(nicInfo.port_id);

    // Now we will wait for all the lcores (except main lcore = 0) to finish before we exit the application.
    for (uint16_t i = 1; i < logicalCores.size(); ++i) {
        std::cout << "Waiting for logical core " << logicalCores[i] << " to join. " << std::endl;
        rte_eal_wait_lcore(logicalCores[i]);
    }

    std::cout << "Exiting DPDK program ... " << std::endl;
    rte_eal_cleanup();
    return 0;
}

int main(int argc, char **argv) {
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = terminate;
    sigaction(SIGTERM, &action, nullptr);
    sigaction(SIGINT, &action, nullptr);

    int32_t return_val = rte_eal_init(argc, argv);
    if (return_val < 0) {
        spdlog::error("EAL: Unable to initialize DPDK EAL. Error code: {}",rte_errno);
        exit(1);
    }
    argc -= return_val;
    argv += return_val;


    BenchParam benchParam;
    parse_args(argc, argv, benchParam);
    // return cross_core_call(argc, argv, benchParam);
    return direct_nic_call(argc, argv, benchParam);
}
