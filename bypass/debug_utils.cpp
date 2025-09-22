#include "debug_utils.h"

#include <stdint.h>
#ifdef _WIN32
#include <intrin.h>
#pragma intrinsic(__rdtsc)
#else
#include <x86intrin.h>
#endif

uint64_t get_cycles() {
#ifdef _WIN32
    return __rdtsc();
#else
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
#endif
}


void dump_mem_hex(const void* addr, size_t len) {
    printf_error("Address: %p\n", addr);

    size_t print_len = len < 16 ? len : 16;
    const uint8_t* p = static_cast<const uint8_t*>(addr);

    printf_error("%zu bytes hex:\n", print_len);
    for (size_t i = 0; i < print_len; ++i) {
        printf_error("%02X ", p[i]);
    }
    printf_error("\n");
}
