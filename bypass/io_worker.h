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
    IOProcess():m_dataQueue(nullptr){}
    virtual ~IOProcess(){
        delete m_dataQueue;
        m_dataQueue = nullptr;
    }
    bool InitPrimaryResource(const std::string poolname,
                             const std::string ringname) ;

    int IO_loop();
    int MainLoop() { return IO_loop(); }

private:
    void write_packet(rte_mbuf *packet) ;
    std::queue<int> tasks;

    ExchangeQueue* m_dataQueue;
};

class NicProcess {
public:
    NicProcess():m_attachDataQueue(nullptr){}
    virtual ~NicProcess(){
        delete m_attachDataQueue;
        m_attachDataQueue = nullptr;
    }
    bool InitNicResource(const std::string ringname) ;
    int CMD_loop() ;
    int MainLoop() { return CMD_loop(); }

private:
    uint64_t m_total_rx_packets = 0;
    LiteQueue *m_attachDataQueue;
};
#endif //IO_WORKDER_H
