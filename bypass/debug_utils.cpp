#include "debug_utils.h"

#include <stdint.h>
#ifdef _WIN32
#include <intrin.h>
#pragma intrinsic(__rdtsc)
#else
#include <x86intrin.h>
#endif

#include <sys/timerfd.h>
#include <unistd.h>

volatile sig_atomic_t exit_indicator = 0;
void terminate(int signal) { exit_indicator = 1; }

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

void Stats::Init(){
    lastCount=0;
    totalCount=0;
    statisticTimer.reset();
}
void Stats::Ticks(){
    uint64_t period=statisticTimer.nanoSeconds();
    uint64_t doneCount = totalCount - lastCount;
    lastCount=totalCount;
    statisticTimer.reset();
    spdlog::info("Stats: iops {:.2f} kpps", (double)(doneCount)/(double)(period/1000000));
}

std::string get_current_data_time() {
    // Example of the very popular RFC 3339 format UTC time
    std::time_t time = std::time({});
    char timeString[std::size("yyyy-mm-ddThh:mm:ssZ")];
    std::strftime(std::data(timeString), std::size(timeString), "%FT%TZ",
                  std::gmtime(&time));
    return timeString;
}

std::atomic<int> counter(0);

void timerThread(void *pstats) {
    int tfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (tfd == -1) {
        return;
    }

    itimerspec new_value{};
    new_value.it_interval.tv_sec = 1;  // Trigger interval: 1 second
    new_value.it_value.tv_sec = 1;     // For the next trigger.
    timerfd_settime(tfd, 0, &new_value, nullptr);

    while (!exit_indicator) {
        uint64_t expirations;
        read(tfd, &expirations, sizeof(expirations));  // Block until to the expiraton
        counter += expirations;
        if (pstats) {
            Stats *ps = static_cast<Stats *>(pstats);
            ps->Ticks();
        }
    }

    close(tfd);
}
