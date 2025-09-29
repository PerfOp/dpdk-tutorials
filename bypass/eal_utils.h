#ifndef BPBUF_UTILS_H
#define BPBUF_UTILS_H

#include <rte_eal.h>
#include <rte_errno.h>
#include <rte_ethdev.h>
#include <rte_mbuf.h>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "debug_utils.h"

const uint16_t KSHARE_MBUF_SIZE = 4 * 1024;
bool check_device_offloading_support(const uint16_t portId,
                                     rte_eth_dev_info& devInfo);

constexpr uint16_t NIC_STATISTICS_INTERVAL_MSEC = 1000;  // 1 seconds.
int get_and_print_nic_statistics(const uint16_t port_id);

class NicInfo {
public:
    std::string port_pci;
    uint16_t port_id;
    struct rte_ether_addr mac;
    rte_eth_dev_info devInfo;

    int32_t ValidatePci(const std::string& name, rte_mempool* memory_pool,
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

    int32_t StopPci(){
        rte_eth_dev_stop(port_id);
        rte_eth_dev_close(port_id);
    }
    uint8_t* GetRawMac() { return mac.addr_bytes; }
};

class DynaQueue {
public:
    DynaQueue() : m_dynfieldoffset(0) {
        if (!attach_dynfield_to_mbuf()) {
            exit(1);
        }
    };
    inline int get_offset() { return m_dynfieldoffset; }
    bool attach_dynfield_to_mbuf();

private:
    int m_dynfieldoffset;
};

typedef struct sGlobalHandle {
    uint16_t nic_port_id;
} GlobalHandle;

typedef struct sMemPool {
    std::string pool_name{""};
    rte_mempool* pool_handle{nullptr};
    bool create_pool(std::string name, int count, int buffersize, int port) {
        pool_name = name;
        pool_handle = rte_pktmbuf_pool_create(
            pool_name.c_str(),  // Name of memory buffer pool.
            count,              // Count of buffers in pool. (2048 - 1 = 2047)
            RTE_MEMPOOL_CACHE_MAX_SIZE,  // Mempool cache size on cpu, 0: do not
                                         // use cache.
            0,           // Size of private area of memory buffer.
            buffersize,  // Size of memory buffer.
            port);       // Socket on which memory buffer is created.

        if (!pool_handle) {
            spdlog::info("Unable to create a new pool {}. rte errno: {}",
                         pool_name.c_str(), rte_strerror(rte_errno));
            rte_eal_cleanup();
            exit(1);
        } else {
            spdlog::info("Pool {} : {} buffers {} bytes/per buffer.", pool_name,
                         count, buffersize);
        }

        return true;
    }

    bool attach_pool(std::string name) {
        pool_name = name;
        pool_handle = rte_mempool_lookup(
            pool_name.c_str());  // Name of memory buffer pool.

        if (!pool_handle) {
            spdlog::info("Unable to attach the pool {}. rte errno: {}",
                         pool_name.c_str(), rte_strerror(rte_errno));
            rte_eal_cleanup();
            exit(1);
        } else {
            spdlog::info("Pool {} attached.", pool_name);
        }
        return true;
    }

    rte_mbuf* const allocate_mbuf() {
        if (pool_handle == nullptr) {
            spdlog::error("Does not allocated valid pool with name:{}",
                          pool_name);
            return nullptr;
        }
        rte_mbuf* const packet = rte_pktmbuf_alloc(pool_handle);
        if (!packet) {
            spdlog::error("Unable to alloc a buffer for packet!");
        }
        return packet;
    }
} MemPool;

typedef struct sRingBuf {
    std::string ring_name{""};
    rte_ring* ring_handle{nullptr};
    bool create_ring(std::string name, uint32_t capacity, int port) {
        if (name.empty()) {
            spdlog::error("Please input valid ring name");
            return false;
        }
        if (ring_handle) {
            spdlog::error(
                "Ring was Assigned a valid handle, do not reinitialize {}!",
                name);
            return false;
        }
        ring_name = name;
        ring_handle = rte_ring_create(
            ring_name.c_str(),  // Name of ring buffer.
            capacity,  // Max size of ring buffer. (512 - 1 = 511 elements)
            port,      // Socket on which ring buffer will be created.
            (RING_F_SP_ENQ | RING_F_SC_DEQ));  // Ring buffer type is Single
                                               // producer / Single consumer.

        if (!ring_handle) {
            spdlog::error("Unable to create ring: {} RTE error: {}", ring_name,
                          rte_strerror(rte_errno));
            rte_eal_cleanup();
            exit(1);
        } else {
            spdlog::info("Ring created: {} with {} elements.", ring_name,
                         capacity);
        }
        return true;
    }
    bool attach_ring(std::string name) {
        ring_handle = rte_ring_lookup(name.c_str());
        if (ring_handle == nullptr) {
            spdlog::error("Unable to attach for ring buffer: {} RTE error:{}",
                          name, rte_strerror(rte_errno));
            rte_eal_cleanup();
            exit(1);
        } else {
            ring_name = name;
            spdlog::info("Attached ring buffer: {}", ring_name);
        }
        return true;
    }

    bool produce_packets(rte_mbuf* packet, uint16_t burst) {
        if (!rte_ring_enqueue(ring_handle, packet)) {
            return true;
        } else {
            rte_pktmbuf_free(packet);
            return false;
        }
    }
    uint32_t consume_packets(rte_mbuf** packets, uint32_t burst) {
        uint32_t rx_count = rte_ring_dequeue_burst(
            ring_handle, reinterpret_cast<void**>(packets), burst, nullptr);
        return rx_count;
    }
} RingBuf;

#endif  // BPBUF_UTILS_H
