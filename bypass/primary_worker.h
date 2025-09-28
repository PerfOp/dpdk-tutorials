#ifndef PRIMARY_WORKER_H
#define PRIMARY_WORKER_H

#include <csignal>

#include "args.h"
#include "debug_utils.h"
#include "eal_utils.h"
#include "time_helper.h"

bool check_device_offloading_support(const uint16_t portId,
                                     rte_eth_dev_info &devInfo) ;
class PrimaryProcess {
public:
    PrimaryProcess() : m_pHandleZone(nullptr), m_portId(0xFFFF) {}
    virtual ~PrimaryProcess() {}
    bool InitPrimaryResource(BenchParam& benchparam);

    int MainLoop() {
        scan_request_loop();
        return io_loop();
    }

private:
    bool init_pool_and_ring();
    bool init_nics(BenchParam& benchparam);
    int scan_request_loop();
    int io_loop();
    void write_packet(rte_mbuf* packet, const uint64_t& count);

    DynaQueue m_dataQueue;

    const rte_memzone* m_pHandleZone;
    Stats ioStats;
    MemPool m_dataPool;
    RingBuf m_dataRing;

    MemPool m_cmdPool;
    RingBuf m_cmdRing;

    MemPool m_nicPool;
    RingBuf m_nicRing;

    uint16_t m_portId;
};

#endif  // PRIMARY_WORKDER_H
