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

#include <csignal>
#include "bpbuf_utils.h"
#include "debug_utils.h"
#include "time_helper.h"
#include "base_worker.h"

class NicProcess:public BaseProcess {
public:
    NicProcess():m_portId(0),m_pHandleZone(nullptr) {}
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
