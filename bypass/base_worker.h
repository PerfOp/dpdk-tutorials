/**
 * Copyright (C) The software Authors. All rights reserved.
 * File Name: bypass/base_worker.h
 * Author:
 * mail:
 * Created Time: Thu Sep 25 01:13:02 2025
 * Brief:
 */

#ifndef BASE_WORKER_H
#define BASE_WORKER_H

#include "eal_utils.h"
// #include "debug_utils.h"
// #include "time_helper.h"

class BaseProcess {
public:
    BaseProcess() {}
    virtual ~BaseProcess() {}
    bool InitResource();

protected:
    DynaQueue m_attachDataQueue;

    MemPool m_dataPool;
    RingBuf m_dataRing;

    MemPool m_cmdPool;
    RingBuf m_cmdRing;
};

#endif // BASE_WORKER_H
