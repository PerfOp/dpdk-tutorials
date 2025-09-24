#ifndef IO_WORKER_H
#define IO_WORKER_H

#include <csignal>
#include "bpbuf_utils.h"
#include "debug_utils.h"
#include "time_helper.h"

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

    DynaQueue m_dataQueue;

    Stats ioStats;
    MemPool m_dataPool;
    RingBuf m_dataRing;

    MemPool m_cmdPool;
    RingBuf m_cmdRing;
};

#endif  // IO_WORKDER_H
