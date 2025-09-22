#ifndef BPBUF_UTILS_H
#define BPBUF_UTILS_H

#include <rte_eal.h>
#include <rte_errno.h>
#include <rte_ethdev.h>
#include <rte_mbuf.h>

#include <iostream>
#include <chrono>
#include <string>
#include <thread>

#include "debug_utils.h"

const uint16_t KSHARE_MBUF_SIZE = 4 * 1024;

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

typedef struct sMemPool {
    std::string pool_name{""};
    rte_mempool* pool_handle{nullptr};
    bool create_pool(std::string name, int port) {
        pool_name = name;
        pool_handle = rte_pktmbuf_pool_create(
            pool_name.c_str(),  // Name of memory buffer pool.
            2048,               // Size of memory buffer pool. (2048 - 1 = 2047)
            RTE_MEMPOOL_CACHE_MAX_SIZE,  // Mempool cache size.
            0,  // Size of private area of memory buffer.
            RTE_MBUF_DEFAULT_BUF_SIZE,  // Size of memory buffer.
            port);  // Socket on which memory buffer is created.

        if (!pool_handle) {
            printf_error("Unable to create a new pool %s. rte errno: %s\n",
                         pool_name.c_str(), rte_strerror(rte_errno));
            rte_eal_cleanup();
            exit(1);
        } else {
            printf_error("Create the pool with name: %s\n", pool_name.c_str());
        }
        return true;
    }

    rte_mbuf* const allocate_mbuf() {
        if (pool_handle == nullptr) {
            return nullptr;
        }
        rte_mbuf* const packet = rte_pktmbuf_alloc(pool_handle);
        if (!packet) {
            bypass_log_error("Unable to allocate memory buffer. \n");
        }
        return packet;
    }
} MemPool;

typedef struct sRingBuf {
    std::string ring_name{""};
    rte_ring* ring_handle{nullptr};
    bool create_ring(std::string name, uint32_t capacity, int port) {
        if (name.empty()) {
            bypass_log_error("Please input valid ring name: %s\n",
                             name.c_str());
            return false;
        }
        if (ring_handle) {
            bypass_log_error(
                "Ring was Assigned a valid handle, do not reinitialize %s!\n",
                name.c_str());
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
            bypass_log_error("Unable to create ring buffer: %s RTE error: %s",
                             ring_name.c_str(), rte_strerror(rte_errno));
            rte_eal_cleanup();
            exit(1);
        } else {
            bypass_log_error("Create ring buffer: %s \n", ring_name.c_str());
        }
        return true;
    }
    bool attach_ring(std::string name){
        ring_handle = rte_ring_lookup(name.c_str());
        if (ring_handle == nullptr)
        {
            bypass_log_error("Unable to attach for ring buffer: %s RTE error:%s\n", name.c_str(), rte_strerror(rte_errno));
            rte_eal_cleanup();
            exit(1);
        }else{
            ring_name=name;
            log_info("bypass:","Attached ring buffer: %s\n", ring_name.c_str());
        }
        return true;
    }

    bool produce_packets(rte_mbuf* packet, uint16_t burst){
        if (!rte_ring_enqueue(ring_handle, packet)) {
            return true;
        }else{
            rte_pktmbuf_free(packet);
            return false;
        }
    }
    uint32_t consume_packets(rte_mbuf** packets, uint32_t burst){
        uint32_t rx_count = rte_ring_dequeue_burst(ring_handle, reinterpret_cast<void **>(packets), burst, nullptr);
        return rx_count;
    }
} RingBuf;

#endif  // BPBUF_UTILS_H
