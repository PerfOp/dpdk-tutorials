#include "debug_utils.h"
#include "bpbuf_utils.h"

static const struct rte_mbuf_dynfield tsDynfieldDesc = {
  .name = "dynfield_ts",
  .size = sizeof(uint64_t),
  .align = __alignof__(uint64_t),
};

ExchangeQueue::~ExchangeQueue(){
    if(m_ownedring){
        rte_ring_free(m_ringhandle);
        m_ringhandle=nullptr;
    }
}

bool DynaQueue::attach_dynfield_to_mbuf(){
    m_dynfieldoffset = rte_mbuf_dynfield_register(&tsDynfieldDesc);
    if (m_dynfieldoffset < 0) {
        printf_error("Cannot register mbuf dynfield: dynfield_ts. RTE Errno: %s\n",rte_strerror(rte_errno));
        rte_eal_cleanup();
        exit(1);
    } else {
        printf_error("Timestamp dynamic field offset: %d\n", m_dynfieldoffset);
        //std::cout << "Timestamp dynamic field offset: " << timestamp_dynfield_offset << std::endl;
    }
    return true;
}

bool ExchangeQueue::create_pool(std::string name, int port){
    m_poolname=name;
    m_poolhandle = rte_pktmbuf_pool_create(m_poolname.c_str(),     // Name of memory buffer pool.
        2048,                       // Size of memory buffer pool. (2048 - 1 = 2047)
        RTE_MEMPOOL_CACHE_MAX_SIZE, // Mempool cache size.
        0,                          // Size of private area of memory buffer.
        RTE_MBUF_DEFAULT_BUF_SIZE,  // Size of memory buffer.
                                    //KSHARE_BUFFER_SIZE,
        port);           // Socket on which memory buffer is created.

    if (!m_poolhandle) {
        printf_error("Unable to create a new memory buffer pool. rte errno: %s\n", rte_strerror(rte_errno));
        rte_eal_cleanup();
        exit(1);
    }else{
        printf_error("Create the pool with name: %s\n", m_poolname.c_str());
    }
    return true;
}

bool ExchangeQueue::create_ring(std::string name, uint32_t capacity, int port){
    if(name.empty()){
        bypass_log_error("Please input valid ring name: %s\n", name.c_str());
        return false;
    }
    if(m_ringhandle){
        bypass_log_error("Ring was Assigned a valid handle, do not reinitialize %s!\n", name.c_str());
        return false;
    }
    m_ringname=name;
    m_ringhandle = rte_ring_create(m_ringname.c_str(),         // Name of ring buffer.
        capacity,                              // Max size of ring buffer. (512 - 1 = 511 elements)
        port,                  // Socket on which ring buffer will be created.
        (RING_F_SP_ENQ | RING_F_SC_DEQ)); // Ring buffer type is Single producer / Single consumer.

    if (!m_ringhandle) {
        bypass_log_error("Unable to create ring buffer: %s RTE error: %s", m_ringname.c_str() ,rte_strerror(rte_errno));
        rte_eal_cleanup();
        exit(1);
    }else{
        bypass_log_error("Create ring buffer: %s \n", m_ringname.c_str());
        m_ownedring = true;
    }
    return true;
}

bool ExchangeQueue::attach_ring(std::string name){
    m_ringhandle = rte_ring_lookup(name.c_str());
    if (m_ringhandle == nullptr)
    {
        bypass_log_error("Unable to attach for ring buffer: %s RTE error:%s\n", name.c_str(), rte_strerror(rte_errno));
        rte_eal_cleanup();
        exit(1);
    }else{
        log_info("bypass:","Attached ring buffer: %s\n", m_ringname.c_str());
    }
    return true;
}

rte_mbuf *const ExchangeQueue::allocate_mbuf(){
    if(m_poolhandle==nullptr){
        return nullptr;
    }
    rte_mbuf *const packet = rte_pktmbuf_alloc(m_poolhandle);
    if (!packet) {
        bypass_log_error("Unable to allocate memory buffer. \n");
    }
    return packet;
}

bool ExchangeQueue::produce_packets(rte_mbuf* packet, uint16_t burst){
    if (!rte_ring_enqueue(m_ringhandle, packet)) {
        return true;
    }else{
        rte_pktmbuf_free(packet);
        return false;
    }
}

uint32_t LiteQueue::consume_packets(rte_mbuf** packets, uint32_t burst){
    uint32_t rx_count = rte_ring_dequeue_burst(m_ringhandle, reinterpret_cast<void **>(packets), burst, nullptr);
    return rx_count;
}

bool LiteQueue::attach_ring(std::string name){
    m_ringhandle = rte_ring_lookup(name.c_str());
    if (m_ringhandle == nullptr)
    {
        bypass_log_error("Unable to attach for ring buffer: %s RTE error:%s\n", name.c_str(), rte_strerror(rte_errno));
        rte_eal_cleanup();
        exit(1);
    }else{
        log_info("bypass:","Attached ring buffer: %s\n", m_ringname.c_str());
    }
    return true;
}
