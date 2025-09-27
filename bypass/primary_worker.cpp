#include "primary_worker.h"

#include <spdlog/fmt/bin_to_hex.h>
#include <stdint.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include <cstring>
#include <ctime>
#include <thread>

#include "config.h"
#include "nic_worker.h"

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

bool PrimaryProcess::init_pool_and_ring() {
    // Primary process with look up for the ring buffer and receive the
    // packets generated by primary DPDK application. Lookup for the ring
    if (!m_dataPool.create_pool(KDataPoolName, 1024 * 1024, 1024,
                                rte_socket_id())) {
        exit(1);
    }

    if (!m_dataRing.create_ring(KDataRingName, 512 * 1024, rte_socket_id())) {
        exit(1);
    }

    // CmdPool: 1024 buffers, 512 bytes per buffer
    if (!m_cmdPool.create_pool(KCmdPoolName, 1024, 512, rte_socket_id())) {
        exit(1);
    }

    if (!m_cmdRing.create_ring(KCmdRingName, 512 * 1024, rte_socket_id())) {
        exit(1);
    }

    // NicPool: 1024*1024 buffers, 1536 bytes per buffer
    if (!m_nicPool.create_pool(KNicPoolName, KNicBufCount, 1536,
                               rte_socket_id())) {
        exit(1);
    }

    if (!m_nicRing.create_ring(KNicRingName, 512 * 1024, rte_socket_id())) {
        exit(1);
    }

    m_pHandleZone = rte_memzone_reserve(KHandlerZone.c_str(),
                                        sizeof(GlobalHandle), SOCKET_ID_ANY, 0);
    if (m_pHandleZone == nullptr) {
        spdlog::error("Failed to reserve memzone: {}", KHandlerZone);
        exit(1);
    }
    return true;
}

bool PrimaryProcess::init_nics(BenchParam &benchparam) {
    uint16_t port_ids[RTE_MAX_ETHPORTS] = {0};
    int16_t id = 0;
    int16_t total_port_count = 0;
    int32_t return_val = 0;

    // Detecting the available ports (ethernet interfaces) in the system.
    RTE_ETH_FOREACH_DEV(id) {
        port_ids[total_port_count] = id;
        total_port_count++;
        if (total_port_count >= RTE_MAX_ETHPORTS) {
            std::cerr
                << "Total number of detected ports exceeds RTE_MAX_ETHPORTS. "
                << std::endl;
            rte_eal_cleanup();
            exit(1);
        }
    }

    if (total_port_count == 0) {
        std::cerr << "No ports detected in the system. " << std::endl;
        rte_eal_cleanup();
        exit(1);
    }

    spdlog::info("Total ports detected: {}", total_port_count);

    uint16_t output_port_id =
        std::numeric_limits<decltype(output_port_id)>::max();
    if (rte_eth_dev_get_port_by_name(benchparam.port_pci.c_str(),
                                     &output_port_id)) {
        spdlog::error("Unable to get port id against port: {}",
                      benchparam.port_pci);
    }

    struct rte_ether_addr mac;
    rte_eth_macaddr_get(output_port_id, &mac);
    memcpy(benchparam.src_mac, mac.addr_bytes, sizeof(mac.addr_bytes));

    // Check about the RX/TX offloading support of current ethernet device.
    // A ethernet device from different vendors (Intel, Nvidia, Broadcom etc.)
    // supports different Rx/Tx offloading capabilities. So we first check which
    // Rx/Tx offloading capabilities are supported by our ether device.
    rte_eth_dev_info devInfo;
    if (!check_device_offloading_support(output_port_id, devInfo)) {
        rte_eal_cleanup();
        exit(1);
    }

    // Detecting the logical cores (CPUs) ids passed to this DPDK application.
    // uint16_t i = 0;
    // std::vector<uint16_t> logicalCores;
    // std::cout << "Logical cores ids (CPU ids): ";
    // RTE_LCORE_FOREACH(i) {
    // logicalCores.push_back(i);
    // std::cout << i << " ";
    // }
    // std::cout << std::endl;

    // // We must have atleast two logical cores passed as an argument to this
    // DPDK application. The first logical core will get and print the nic
    // statistics.
    // // The second logical core will execute the packet transmission routine.
    // if (logicalCores.size() != 2)
    // {
    // std::cerr << "Two logical cores are required to run this DPDK
    // application. " << std::endl; rte_eal_cleanup(); exit(1);
    // }

    // // Creating memory pool which contains the memory buffers. A memory
    // buffer is the buffer where DPDK driver will write an
    // // incoming packet. Below memory pool has name "mempool_1" and has 65535
    // available memory buffer. A single memory buffer
    // // has a size of RTE_MBUF_DEFAULT_BUF_SIZE (2048Bytes + 128Bytes).
    // rte_mempool *memory_pool =
    // rte_pktmbuf_pool_create(MEMORY_POOL_NAME.c_str(), MEMORY_POOL_SIZE, 512,
    // 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

    // Configuring the port (ethernet interface). An ethernet interface can have
    // multiple receive queues and transmit queues. Currently we are setting up
    // one transmit queue and no receive queue as we are not receiving packets
    // in this tutorial.
    const uint16_t rx_queues = 0;
    const uint16_t tx_queues = 1;

    rte_eth_conf portConf = {
        .rxmode = {.mq_mode = RTE_ETH_MQ_RX_NONE},
        .txmode = {.mq_mode = RTE_ETH_MQ_TX_NONE,
                   .offloads = (devInfo.tx_offload_capa &
                                RTE_ETH_TX_OFFLOAD_IPV4_CKSUM)}};

    // Configure the port (ethernet interface).
    if ((return_val = rte_eth_dev_configure(output_port_id, rx_queues,
                                            tx_queues, &portConf)) != 0) {
        spdlog::error(
            "Unable to configure port. port Id:{} "
            "Return code: {}",
            output_port_id, return_val);
        rte_eal_cleanup();
        exit(1);
    } else {
        spdlog::warn("Config the nic done {}!", output_port_id);
    }

    const int16_t portSocketId = rte_eth_dev_socket_id(output_port_id);
    const int16_t coreSocketId = rte_socket_id();

    // Configure the Rx queue(s) of the port.
    for (uint16_t i = 0; i < rx_queues; i++) {
        return_val = rte_eth_rx_queue_setup(
            output_port_id, i, 256,
            ((portSocketId >= 0) ? portSocketId : coreSocketId), nullptr,
            m_nicPool.pool_handle /*memory_pool*/);

        if (return_val < 0) {
            spdlog::error(
                "Unable to setup RX queue port Id:{} "
                "Return code: {}",
                output_port_id, return_val);
            rte_eal_cleanup();
            exit(1);
        }
    }

    // Configure the Tx queue(s) of the port.
    for (uint16_t i = 0; i < tx_queues; i++) {
        return_val = rte_eth_tx_queue_setup(
            output_port_id, i, 1024,
            ((portSocketId >= 0) ? portSocketId : coreSocketId), nullptr);

        if (return_val < 0) {
            spdlog::error(
                "Unable to setup TX queue port Id:{} "
                "Return code: {}",
                output_port_id, return_val);
            rte_eal_cleanup();
            exit(1);
        }
    }

    // Enable promiscuous mode on the port. Not all the DPDK drivers provide the
    // functionality to enable promiscuous mode. So we are going to ignore the
    // result if the API fails.
    return_val = rte_eth_promiscuous_enable(output_port_id);
    if (return_val < 0) {
        std::cout << "Warning: Unable to set the promiscuous mode for port Id: "
                  << output_port_id << " Return code: " << return_val
                  << " Ignoring ... " << std::endl;
    }

    // All the configuration is done. Finally starting the port (ethernet
    // interface) so that we can start transmitting the packets.
    return_val = rte_eth_dev_start(output_port_id);
    if (return_val < 0) {
        std::cout << "Unable to start port Id: " << output_port_id
                  << " Return code: " << return_val << std::endl;
        rte_eal_cleanup();
        exit(1);
    }

    std::cout << "Port configuration successful. Port Id: " << output_port_id
              << std::endl;
    *((uint16_t *)m_pHandleZone->addr) = output_port_id;

    // Prepare memory pool.
    if (!prepare_memory_pool(m_nicPool.pool_handle, benchparam)) {
        spdlog::error("Cannot init the pool for nic");
        rte_eth_dev_stop(output_port_id);
        rte_eth_dev_close(output_port_id);
        rte_eal_cleanup();
        exit(1);
    } else {
        spdlog::warn("Init the pool for nic done");
    }
    /*
        if (!prepare_memory_pool()) {
            rte_eth_dev_stop(output_port_id);
            rte_eth_dev_close(output_port_id);
            rte_eal_cleanup();
            exit(1);
        }

        // Now initiating packet transmission routine on the second logical core
       id. PacketTransmissionThreadParams *packetTransmissionThreadParams = new
       PacketTransmissionThreadParams; packetTransmissionThreadParams->port_id =
       output_port_id; packetTransmissionThreadParams->queue_id = 0;
        packetTransmissionThreadParams->packets_per_second = packets_per_second;
        if ((return_val = rte_eal_remote_launch(transmit_packets_from_interface,
       reinterpret_cast<void *>(packetTransmissionThreadParams),
       logicalCores[1])) != 0) { std::cerr << "Unable to launch packet
       transmission routine on the logical core: %d. Return code: %d" <<
       logicalCores[1] << return_val << std::endl;
            rte_eth_dev_stop(output_port_id);
            rte_eth_dev_close(output_port_id);
            rte_eal_cleanup();
            exit(1);
        }
        */
    return true;
}

bool PrimaryProcess::InitPrimaryResource(BenchParam &benchparam) {
    return init_pool_and_ring() && init_nics(benchparam);
}

int PrimaryProcess::scan_request_loop() {
    while (!exit_indicator) {
        rte_mbuf *cmd_packets[32];
        // Check for any incoming packets in the ring buffer. We try to
        // dequeue max 32 packets at max at a time.
        uint8_t cmd_count = m_cmdRing.consume_packets(cmd_packets, 1);
        if (cmd_count > 0) {
            rte_pktmbuf_free(cmd_packets[0]);
            break;
        } else {
            spdlog::info("Waiting for the nic thread");
            sleep(1);
        }
    }
    return 1;
}

int PrimaryProcess::io_loop() {
    // We will print the logical core id (CPU id) on which this thread is
    // going to be executed. rte_lcore_id() function will return the current
    // logical core id (CPU id).

    spdlog::info("Starting packet generation routine. Logical core id:{} ",
                 rte_lcore_id());

    // ioStats.Init();
    std::thread st(timerThread, &this->ioStats);
    while (!exit_indicator) {
        // using namespace std::literals;
        // std::this_thread::sleep_for(1ms);
        // rte_mbuf *const packet = m_dataPool->allocate_mbuf();
        rte_mbuf *const packet = m_nicPool.allocate_mbuf();
        if (!packet) {
            continue;
        }

        //this->write_packet(packet, ioStats.totalCount);

        // Enqueuing the packet in the ring buffer.
        if (m_dataRing.produce_packets(packet, 1)) {
            ioStats.totalCount++;
            // if (!(ioStats.totalCount % 1000)) {
            // std::cout << "Enqueued packet(s) in the ring. total :" <<
            // ioStats.totalCount << std::endl;
            // }
        } else {
            // std::cerr << "Space is full. "<< std::endl;
            rte_pktmbuf_free(packet);
        }
    }
    st.join();

    spdlog::info("Total packets generated: {}", ioStats.totalCount);
    return 0;
}

void PrimaryProcess::write_packet(rte_mbuf *packet, const uint64_t &count) {
    // Timestamp the memory buffer (packet). The timestamp will be written in
    // the head room of the memory buffer. Head room is the memory area before
    // actual data room.
    static timespec ts{0};
    clock_gettime(CLOCK_REALTIME, &ts);
    *(RTE_MBUF_DYNFIELD(packet,
                        m_dataQueue.get_offset() /*timestamp_dynfield_offset*/,
                        uint64_t *)) = ((ts.tv_sec * 1000000000L) + ts.tv_nsec);

    // Filling some data in the packet.
    // static const char data[] = "A quick brown fox jumps over the lazy dog.";
    std::string data =
        "A quick brown fox jumps over the lazy dog." + std::to_string(count);
    uint8_t *const data_ptr = rte_pktmbuf_mtod(packet, uint8_t *);
    std::memcpy(data_ptr, data.c_str(), data.size());
    packet->data_len = data.size();
}

