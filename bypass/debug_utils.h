#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <csignal>
#include <spdlog/spdlog.h>
#include "time_helper.h"

#define SHALLOW_STR_OF(x) #x
#define STR_OF(x) SHALLOW_STR_OF(x)

#define ALIGN_DOWN_BY(length, alignment) \
    ((ULONG_PTR)(length)& ~(alignment - 1))
#define ALIGN_UP_BY(length, alignment) \
    (ALIGN_DOWN_BY(((ULONG_PTR)(length)+alignment - 1), alignment))

#define STRUCT_FIELD_OFFSET(structPtr, field) \
    ((UCHAR *)&(structPtr)->field - (UCHAR *)(structPtr))

#define printf_error(...) \
    fprintf(stderr, __VA_ARGS__)

#define printf_verbose(format, ...) \
    if (logVerbose) { LARGE_INTEGER Qpc; QueryPerformanceCounter(&Qpc); printf("Qpc=%llu " format, Qpc.QuadPart, __VA_ARGS__); }

#define ABORT(...) \
    printf_error(__VA_ARGS__); exit(1)

#define ASSERT_FRE(expr) \
    if (!(expr)) { ABORT("(%s) failed line %d\n", #expr, __LINE__);}

#if DBG
#define VERIFY(expr) assert(expr)
#else
#define VERIFY(expr) (expr)
#endif

#define log_info(prefix, fmt, ...) \
    fprintf(stderr, "%s: " fmt "\n", prefix, ##__VA_ARGS__)

#define log_error(prefix, fmt, ...) \
    fprintf(stderr, "%s: " fmt "\n", prefix, ##__VA_ARGS__)

#define bypass_log_error(fmt, ...) \
    fprintf(stderr, "bypass: " fmt "\n", ##__VA_ARGS__)

// #ifdef DEBUG
// #define log_info(prefix, fmt, ...) \
    // fprintf(stderr, "%s: " fmt "\n", prefix, ##__VA_ARGS__)
// #else
// #define log_info(prefix, fmt, ...)
// #endif

void dump_mem_hex(const void* addr, size_t len);
uint64_t get_cycles();

extern std::atomic<bool> exit_indicator;
void terminate(int signal);

typedef struct sStats{
    ElapsedTime statisticTimer;
    uint64_t lastCount;
    uint64_t totalCount;
    void Init();
    void Ticks();
}Stats;

int verify_mtu(uint8_t* packet) ;

void timerThread(void *pstats);
std::string get_current_data_time() ;

#endif //DEBUG_UTILS_H
