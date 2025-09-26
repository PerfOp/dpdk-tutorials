/**
 * Copyright (C) The software Authors. All rights reserved.
 * File Name: bypass/base_worker.cpp
 * Author:
 * mail:
 * Created Time: Thu Sep 25 01:13:06 2025
MOD: Adding new class for worker.* Brief:
 */
#include "base_worker.h"
#include "config.h"

bool BaseProcess::InitResource() {
    // Register a timestamp dynamic field.
    // m_attachDataQueue = new DynaQueue();
    if (!m_dataPool.attach_pool(KDataPoolName)) {
        exit(1);
    }

    if (!m_dataRing.attach_ring(KDataRingName)) {
        exit(1);
    }

    if (!m_cmdPool.attach_pool(KCmdPoolName)) {
        exit(1);
    }

    if (!m_cmdRing.attach_ring(KCmdRingName)) {
        exit(1);
    }

    return true;
}
