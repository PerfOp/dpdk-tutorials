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
#include "base_worker.h"
#include "bpbuf_utils.h"
#include "debug_utils.h"
#include "time_helper.h"

class MemProcess :public BaseProcess{
public:
    MemProcess() {}
    virtual ~MemProcess() {}
    bool InitMemResource();
    int MainLoop() { return recv_loop(); }

private:
    int recv_loop();
    void issue_request();
    uint64_t m_total_rx_packets = 0;

    Stats nicStats;
};

#endif // NIC_WORKER_H
