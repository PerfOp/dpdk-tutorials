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
        log_error("bypass", "Sent %llu packets\n", doneCount);
    }
}Stats;

class IOProcess {
public:
    IOProcess() {}
    virtual ~IOProcess() {}
    bool InitPrimaryResource();

    int IO_loop();
    int MainLoop() { return IO_loop(); }

private:
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
    int CMD_loop();
    int MainLoop() { return CMD_loop(); }

private:
    uint64_t m_total_rx_packets = 0;
    // LiteQueue *m_attachDataQueue;
    DynaQueue m_attachDataQueue;

    MemPool m_dataPool;
    RingBuf m_dataRing;

    MemPool m_cmdPool;
    RingBuf m_cmdRing;
};
#endif  // IO_WORKDER_H
