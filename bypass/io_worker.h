#ifndef IO_WORKER_H
#define IO_WORKER_H

#include <csignal>
#include <cstring>
#include <ctime>
#include <iostream>
#include <queue>
#include <vector>

#include "bpbuf_utils.h"
#include "debug_utils.h"
#include "time_helper.h"

extern volatile sig_atomic_t exit_indicator;

typedef struct sStats{
    ElapsedTime statisticTimer;
    uint64_t lastCount;
    uint64_t totalCount;
    void Init(){
        lastCount=0;
        totalCount=0;
        statisticTimer.reset();
    }
    void Ticks(){
        uint64_t period=statisticTimer.nanoSeconds();
        statisticTimer.reset();
        uint64_t doneCount = totalCount - lastCount;
        lastCount=totalCount;
        log_error("bypass", "Sent %lu packets\n", doneCount);
    }
}Stats;

class IOProcess {
public:
    IOProcess() {}
    virtual ~IOProcess() {}
    bool InitPrimaryResource();

    int MainLoop() {
        scan_request_loop();
        return io_loop();
    }

private:
    int scan_request_loop();
    int io_loop();
    void write_packet(rte_mbuf *packet);
    std::queue<int> tasks;

    DynaQueue m_dataQueue;

    Stats dataStats;
    MemPool m_dataPool;
    RingBuf m_dataRing;

    MemPool m_cmdPool;
    RingBuf m_cmdRing;
};

class NicProcess {
public:
    NicProcess() {}
    virtual ~NicProcess() {}
    bool InitNicResource();
    int MainLoop() { return recv_loop(); }

private:
    int recv_loop();
    void issue_request();
    uint64_t m_total_rx_packets = 0;
    // LiteQueue *m_attachDataQueue;
    DynaQueue m_attachDataQueue;

    MemPool m_dataPool;
    RingBuf m_dataRing;

    MemPool m_cmdPool;
    RingBuf m_cmdRing;
};
#endif  // IO_WORKDER_H
