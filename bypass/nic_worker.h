/**
 * Copyright (C) The software Authors. All rights reserved.
 * File Name: bypass/nic_worker.h
 * Author:
 * mail:
 * Created Time: Wed Sep 24 05:20:07 2025
 * Brief:
 */

#ifndef NIC_WORKER_H
#define NIC_WORKER_H

#include "args.h"
#include "eal_utils.h"
#include "debug_utils.h"
#include "time_helper.h"
#include "base_worker.h"

constexpr uint32_t MEMORY_POOL_SIZE = 65535;                    // Size of the memory pool.
bool prepare_memory_pool(rte_mempool *mempool, const BenchParam& benchparam);
bool preset_memory_pool(uint16_t payloadsize, const BenchParam& benchparam);

class NicProcess:public BaseProcess {
public:
    NicProcess():m_portId(0xFFFF),m_pHandleZone(nullptr) {}
    virtual ~NicProcess() {}
    bool InitNicResource();
    int MainLoop() { return recv_loop(); }

private:
    bool init_nics();
    int recv_loop();
    void issue_request();

    Stats nicStats;

    MemPool m_nicPool;
    RingBuf m_nicRing;

    const struct rte_memzone *m_pHandleZone;
    uint16_t m_portId;
};

#endif // NIC_WORKER_H
