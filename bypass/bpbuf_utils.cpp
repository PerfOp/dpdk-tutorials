#include "bpbuf_utils.h"

static const struct rte_mbuf_dynfield tsDynfieldDesc = {
  .name = "dynfield_ts",
  .size = sizeof(uint64_t),
  .align = __alignof__(uint64_t),
};

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
