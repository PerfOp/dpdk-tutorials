#ifndef PRIMARY_WORKER_H
#define PRIMARY_WORKER_H

#include <csignal>
#include "eal_utils.h"
#include "debug_utils.h"
#include "time_helper.h"
#include "args.h"

class PrimaryProcess {
public:
    PrimaryProcess():m_pHandleZone(nullptr) {}
    virtual ~PrimaryProcess() {}
    bool InitPrimaryResource(const BenchParam& benchparam);

    int MainLoop() {
        scan_request_loop();
        return io_loop();
    }

private:
    bool init_pool_and_ring();
    bool init_nics(const BenchParam& benchparam);
    int scan_request_loop();
    int io_loop();
    void write_packet(rte_mbuf *packet);

    DynaQueue m_dataQueue;

    const rte_memzone *m_pHandleZone;
    Stats ioStats;
    MemPool m_dataPool;
    RingBuf m_dataRing;

    MemPool m_cmdPool;
    RingBuf m_cmdRing;

    MemPool m_nicPool;
    RingBuf m_nicRing;
};

#endif  // PRIMARY_WORKDER_H
