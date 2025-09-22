#ifndef BPBUF_UTILS_H
#define BPBUF_UTILS_H

#include <rte_eal.h>
#include <rte_errno.h>
#include <rte_ethdev.h>
#include <rte_mbuf.h>
#include <chrono>
#include <thread>
#include <string>

const uint16_t KSHARE_MBUF_SIZE=4*1024;

class ExchangeQueue{
public:
    ExchangeQueue():
        m_dynfieldoffset(0),
        m_ownedring(false),
        m_ringname(""),
        m_ringhandle(nullptr),
        m_sharedpool(nullptr)
    {
        if(!attach_dynfield_to_mbuf()){
            exit(1);
        }
    };
    virtual ~ExchangeQueue();

    inline int get_offset(){return m_dynfieldoffset;}

    // inline rte_mempool* get_pool(){return m_sharedpool;}
    bool create_pool(std::string name, int port);
    rte_mbuf *const allocate_mbuf();

    inline rte_ring* get_ring(){return m_ringhandle;}
    bool create_ring(std::string name, uint32_t capacity, int port);
    bool attach_ring(std::string name);
    inline bool owned_ring(){return m_ownedring;};

    bool produce_packets(rte_mbuf* packet, uint16_t burst=1);
    uint32_t consume_packets(rte_mbuf** packet, uint32_t burst);
private:
    bool attach_dynfield_to_mbuf();
    int m_dynfieldoffset;

    std::string m_poolname;
    rte_mempool * m_sharedpool;

    std::string m_ringname;
    rte_ring* m_ringhandle;
    bool m_ownedring;
};

#endif //BPBUF_UTILS_H
