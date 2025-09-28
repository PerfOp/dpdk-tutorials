#include "eal_utils.h"

static const struct rte_mbuf_dynfield tsDynfieldDesc = {
  .name = "dynfield_ts",
  .size = sizeof(uint64_t),
  .align = __alignof__(uint64_t),
};

bool DynaQueue::attach_dynfield_to_mbuf(){
    m_dynfieldoffset = rte_mbuf_dynfield_register(&tsDynfieldDesc);
    if (m_dynfieldoffset < 0) {
        printf_error("Cannot register mbuf dynfield: dynfield_ts. RTE Errno: %s\n",rte_strerror(rte_errno));
        rte_eal_cleanup();
        exit(1);
    } else {
        printf_error("Timestamp dynamic field offset: %d\n", m_dynfieldoffset);
        //std::cout << "Timestamp dynamic field offset: " << timestamp_dynfield_offset << std::endl;
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
