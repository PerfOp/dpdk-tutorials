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

class NicInfo {
public:
    std::string port_pci;
    uint16_t port_id;
    struct rte_ether_addr mac;
    rte_eth_dev_info devInfo;

    int32_t ValidatePci(const std::string& name, rte_mempool* memory_pool,
                        const uint16_t rx_queues, const uint16_t tx_queues);

    int32_t StopPci(){
        rte_eth_dev_stop(port_id);
        rte_eth_dev_close(port_id);
    }
    uint8_t* GetRawMac() { return mac.addr_bytes; }
};


#endif  // BPBUF_UTILS_H
