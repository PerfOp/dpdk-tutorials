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
    NicProcess() {}
    virtual ~NicProcess() {}
    bool InitNicResource();
    int MainLoop() { return recv_loop(); }

private:
    bool initnicport();
    int recv_loop();
    void issue_request();

    Stats nicStats;
};

#endif // NIC_WORKER_H
