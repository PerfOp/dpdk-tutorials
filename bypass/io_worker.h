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

extern volatile sig_atomic_t exit_indicator ;

class IOProcess {
public:
    IOProcess(){}
    virtual ~IOProcess(){
    }
    bool InitPrimaryResource(const std::string poolname,
                             const std::string ringname) ;

    int IO_loop();
    int MainLoop() { return IO_loop(); }

private:
    void write_packet(rte_mbuf *packet) ;
    std::queue<int> tasks;

    DynaQueue m_dataQueue;
    MemPool m_dataPool;
    RingBuf m_dataRing;
};

class NicProcess {
public:
    NicProcess(){}
    virtual ~NicProcess(){
    }
    bool InitNicResource(const std::string ringname) ;
    int CMD_loop() ;
    int MainLoop() { return CMD_loop(); }

private:
    uint64_t m_total_rx_packets = 0;
    //LiteQueue *m_attachDataQueue;
    DynaQueue m_attachDataQueue;

    MemPool m_dataPool;
    RingBuf m_dataRing;
};
#endif //IO_WORKDER_H
