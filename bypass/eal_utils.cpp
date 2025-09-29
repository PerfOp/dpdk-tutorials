#include "eal_utils.h"
#include <iomanip>

static const struct rte_mbuf_dynfield tsDynfieldDesc = {
  .name = "dynfield_ts",
  .size = sizeof(uint64_t),
  .align = __alignof__(uint64_t),
};

bool DynaQueue::attach_dynfield_to_mbuf(){
    m_dynfieldoffset = rte_mbuf_dynfield_register(&tsDynfieldDesc);
    if (m_dynfieldoffset < 0) {
        spdlog::error("Cannot register mbuf dynfield: dynfield_ts. RTE Errno: {}",rte_strerror(rte_errno));
        rte_eal_cleanup();
        exit(1);
    } else {
        spdlog::info("Timestamp dynamic field offset: {}", m_dynfieldoffset);
    }
    return true;
}

bool check_device_offloading_support(const uint16_t portId,
                                     rte_eth_dev_info &devInfo) {
    int32_t ret = rte_eth_dev_info_get(portId, &devInfo);
    if (ret != 0) {
        spdlog::error(
            "Error occurred while getting device info (port {}). Return "
            "code:{} ",
            portId, ret);
        return false;
    }

    // Tx Capabilities
    printf("Tx Offloading Capabilities for ethernet device (port): %d\n",
           portId);
    if (devInfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_VLAN_INSERT) {
        printf("  RTE_ETH_TX_OFFLOAD_VLAN_INSERT");
    }
    if (devInfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_IPV4_CKSUM) {
        printf("  RTE_ETH_TX_OFFLOAD_IPV4_CKSUM");
    }
    if (devInfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_UDP_CKSUM) {
        printf("  RTE_ETH_TX_OFFLOAD_UDP_CKSUM");
    }
    if (devInfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_TCP_CKSUM) {
        printf("  RTE_ETH_TX_OFFLOAD_TCP_CKSUM");
    }
    if (devInfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_SCTP_CKSUM) {
        printf("  RTE_ETH_TX_OFFLOAD_SCTP_CKSUM");
    }
    if (devInfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_TCP_TSO) {
        printf("  RTE_ETH_TX_OFFLOAD_TCP_TSO");
    }
    if (devInfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_UDP_TSO) {
        printf("  RTE_ETH_TX_OFFLOAD_UDP_TSO");
    }
    if (devInfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_OUTER_IPV4_CKSUM) {
        printf("  RTE_ETH_TX_OFFLOAD_OUTER_IPV4_CKSUM");
    }
    if (devInfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_QINQ_INSERT) {
        printf("  RTE_ETH_TX_OFFLOAD_QINQ_INSERT");
    }
    if (devInfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_VXLAN_TNL_TSO) {
        printf("  RTE_ETH_TX_OFFLOAD_VXLAN_TNL_TSO");
    }
    if (devInfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_GRE_TNL_TSO) {
        printf("  RTE_ETH_TX_OFFLOAD_GRE_TNL_TSO");
    }
    if (devInfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_IPIP_TNL_TSO) {
        printf("  RTE_ETH_TX_OFFLOAD_IPIP_TNL_TSO");
    }
    if (devInfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_GENEVE_TNL_TSO) {
        printf("  RTE_ETH_TX_OFFLOAD_GENEVE_TNL_TSO");
    }
    if (devInfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_MACSEC_INSERT) {
        printf("  RTE_ETH_TX_OFFLOAD_MACSEC_INSERT");
    }
    if (devInfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_MT_LOCKFREE) {
        printf("  RTE_ETH_TX_OFFLOAD_MT_LOCKFREE");
    }
    if (devInfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_MULTI_SEGS) {
        printf("  RTE_ETH_TX_OFFLOAD_MULTI_SEGS");
    }
    if (devInfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE) {
        printf("  RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE");
    }
    if (devInfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_SECURITY) {
        printf("  RTE_ETH_TX_OFFLOAD_SECURITY");
    }
    if (devInfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_UDP_TNL_TSO) {
        printf("  RTE_ETH_TX_OFFLOAD_UDP_TNL_TSO");
    }
    if (devInfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_IP_TNL_TSO) {
        printf("  RTE_ETH_TX_OFFLOAD_IP_TNL_TSO");
    }
    if (devInfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_OUTER_UDP_CKSUM) {
        printf("  RTE_ETH_TX_OFFLOAD_OUTER_UDP_CKSUM");
    }
    if (devInfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_SEND_ON_TIMESTAMP) {
        printf("  RTE_ETH_TX_OFFLOAD_SEND_ON_TIMESTAMP");
    }

    printf("\n");

    // Rx Capabilities
    printf("Rx Offloading Capabilities for Port: %d\n", portId);
    if (devInfo.rx_offload_capa & RTE_ETH_RX_OFFLOAD_VLAN_STRIP) {
        printf("  RTE_ETH_RX_OFFLOAD_VLAN_STRIP");
    }
    if (devInfo.rx_offload_capa & RTE_ETH_RX_OFFLOAD_IPV4_CKSUM) {
        printf("  RTE_ETH_RX_OFFLOAD_IPV4_CKSUM");
    }
    if (devInfo.rx_offload_capa & RTE_ETH_RX_OFFLOAD_UDP_CKSUM) {
        printf("  RTE_ETH_RX_OFFLOAD_UDP_CKSUM");
    }
    if (devInfo.rx_offload_capa & RTE_ETH_RX_OFFLOAD_TCP_CKSUM) {
        printf("  RTE_ETH_RX_OFFLOAD_TCP_CKSUM");
    }
    if (devInfo.rx_offload_capa & RTE_ETH_RX_OFFLOAD_TCP_LRO) {
        printf("  RTE_ETH_RX_OFFLOAD_TCP_LRO");
    }
    if (devInfo.rx_offload_capa & RTE_ETH_RX_OFFLOAD_QINQ_STRIP) {
        printf("  RTE_ETH_RX_OFFLOAD_QINQ_STRIP");
    }
    if (devInfo.rx_offload_capa & RTE_ETH_RX_OFFLOAD_OUTER_IPV4_CKSUM) {
        printf("  RTE_ETH_RX_OFFLOAD_OUTER_IPV4_CKSUM");
    }
    if (devInfo.rx_offload_capa & RTE_ETH_RX_OFFLOAD_MACSEC_STRIP) {
        printf("  RTE_ETH_RX_OFFLOAD_MACSEC_STRIP");
    }
    if (devInfo.rx_offload_capa & RTE_ETH_RX_OFFLOAD_VLAN_FILTER) {
        printf("  RTE_ETH_RX_OFFLOAD_VLAN_FILTER");
    }
    if (devInfo.rx_offload_capa & RTE_ETH_RX_OFFLOAD_VLAN_EXTEND) {
        printf("  RTE_ETH_RX_OFFLOAD_VLAN_EXTEND");
    }
    if (devInfo.rx_offload_capa & RTE_ETH_RX_OFFLOAD_SCATTER) {
        printf("  RTE_ETH_RX_OFFLOAD_SCATTER");
    }
    if (devInfo.rx_offload_capa & RTE_ETH_RX_OFFLOAD_TIMESTAMP) {
        printf("  RTE_ETH_RX_OFFLOAD_TIMESTAMP");
    }
    if (devInfo.rx_offload_capa & RTE_ETH_RX_OFFLOAD_SECURITY) {
        printf("  RTE_ETH_RX_OFFLOAD_SECURITY");
    }
    if (devInfo.rx_offload_capa & RTE_ETH_RX_OFFLOAD_KEEP_CRC) {
        printf("  RTE_ETH_RX_OFFLOAD_KEEP_CRC");
    }
    if (devInfo.rx_offload_capa & RTE_ETH_RX_OFFLOAD_SCTP_CKSUM) {
        printf("  RTE_ETH_RX_OFFLOAD_SCTP_CKSUM");
    }
    if (devInfo.rx_offload_capa & RTE_ETH_RX_OFFLOAD_OUTER_UDP_CKSUM) {
        printf("  RTE_ETH_RX_OFFLOAD_OUTER_UDP_CKSUM");
    }
    if (devInfo.rx_offload_capa & RTE_ETH_RX_OFFLOAD_RSS_HASH) {
        printf("  RTE_ETH_RX_OFFLOAD_RSS_HASH");
    }
    if (devInfo.rx_offload_capa & RTE_ETH_RX_OFFLOAD_BUFFER_SPLIT) {
        printf("  RTE_ETH_RX_OFFLOAD_BUFFER_SPLIT");
    }

    printf("\n");
    printf("Max Rx Queues: %u\n", devInfo.max_rx_queues);
    printf("Max Tx Queues: %u\n", devInfo.max_tx_queues);
    printf(
        "----------------------------------------------------------------------"
        "-------\n");

    return true;
}

int get_and_print_nic_statistics(const uint16_t port_id){
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

int32_t NicInfo::ValidatePci(const std::string& name, rte_mempool* memory_pool,
        const uint16_t rx_queues, const uint16_t tx_queues) {
    uint16_t port_ids[RTE_MAX_ETHPORTS] = {0};
    int16_t id = 0;
    int16_t total_port_count = 0;
    int32_t ret_val = 0;

    // 1. Detecting the available ports (ethernet interfaces) in the system.
    RTE_ETH_FOREACH_DEV(id) {
        port_ids[total_port_count] = id;
        total_port_count++;
        if (total_port_count >= RTE_MAX_ETHPORTS) {
            spdlog::error(
                    "Total number of detected ports exceeds "
                    "RTE_MAX_ETHPORTS. ");
            rte_eal_cleanup();
            exit(1);
        }
    }

    if (total_port_count == 0) {
        spdlog::error("No ports detected in the system. ");
        rte_eal_cleanup();
        exit(1);
    } else {
        spdlog::info("Total ports detected: ", total_port_count);
    }

    // 2.1 Getting port_id against PCI address.
    port_pci = std::string(name);
    port_id = std::numeric_limits<decltype(port_id)>::max();
    if (rte_eth_dev_get_port_by_name(port_pci.c_str(), &port_id)) {
        spdlog::error("Unable to get port id against port {} ", port_pci);
        rte_eal_cleanup();
        exit(1);
    }
    // 2.2 Enumerate the offloading support on the NIC
    if (!check_device_offloading_support(port_id, devInfo)) {
        spdlog::error("Failed to check_device_offloading_support!");
        rte_eal_cleanup();
        exit(1);
    }
    // 2.3 Load the mac address from NIC.
    rte_eth_macaddr_get(port_id, &mac);

    // 3. Configuring the port (ethernet interface). An ethernet interface can
    // have multiple receive queues and transmit queues. Currently we are
    // setting up one transmit queue and no receive queue as we are not
    // receiving packets in this tutorial.
    rte_eth_conf portConf = {
        .rxmode = {.mq_mode = RTE_ETH_MQ_RX_NONE},
        .txmode = {.mq_mode = RTE_ETH_MQ_TX_NONE,
            .offloads = (devInfo.tx_offload_capa &
                    RTE_ETH_TX_OFFLOAD_IPV4_CKSUM)}};
    // Configure the port (ethernet interface).
    if ((ret_val = rte_eth_dev_configure(port_id, rx_queues, tx_queues,
                    &portConf)) != 0) {
        spdlog::error("Unable to configure port Id:{} Return code:{}",
                port_id, ret_val);
        rte_eal_cleanup();
        exit(1);
    }

    // 4.1 portSocketId coreSocketId setup.
    const int16_t portSocketId = rte_eth_dev_socket_id(port_id);
    const int16_t coreSocketId = rte_socket_id();

    // 4.2 Configure the Rx queue(s) of the port.
    for (uint16_t i = 0; i < rx_queues; i++) {
        ret_val = rte_eth_rx_queue_setup(
                port_id, i, 256,
                ((portSocketId >= 0) ? portSocketId : coreSocketId), nullptr,
                memory_pool);

        if (ret_val < 0) {
            spdlog::error("Unable to setup RX queue {} Return code {}", i,
                    ret_val);
            rte_eal_cleanup();
            exit(1);
        }
        spdlog::info(
                "Port Id: {} Rx Queues: {} setup successful. Socket id: {}",
                port_id, i, (portSocketId >= 0) ? portSocketId : coreSocketId);
    }

    // 4.3 Configure the Tx queue(s) of the port.
    for (uint16_t i = 0; i < tx_queues; i++) {
        ret_val = rte_eth_tx_queue_setup(
                port_id, i, 1024,
                ((portSocketId >= 0) ? portSocketId : coreSocketId), nullptr);

        if (ret_val < 0) {
            spdlog::error("Unable to setup TX queue {} Return code {}", i,
                    ret_val);
            rte_eal_cleanup();
            exit(1);
        }
        spdlog::info(
                "Port Id: {} Tx Queues: {} setup successful. Port socket id: "
                "{} Core socket id: {}",
                port_id, i, portSocketId, coreSocketId);
    }

    // 5. Enable promiscuous mode on the port. Not all the DPDK drivers provide
    // the functionality to enable promiscuous mode. So we are going to
    // ignore the result if the API fails.
    ret_val = rte_eth_promiscuous_enable(port_id);
    if (ret_val < 0) {
        spdlog::warn(
                "Unable to set the promiscuous mode for port Id {} Return code "
                "{}",
                port_id, ret_val);
    }

    // All the configuration is done. Finally starting the port (ethernet
    // interface) so that we can start transmitting the packets.
    ret_val = rte_eth_dev_start(port_id);
    if (ret_val < 0) {
        spdlog::error("Unable to start port Id {} Return code {}", port_id,
                ret_val);
        rte_eal_cleanup();
        exit(1);
    }
    return ret_val;
}


