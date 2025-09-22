#include "time_helper.h"

uint64_t MonoTime::NanoSeconds(void)
{
    struct timespec tspec;
    clock_gettime(CLOCK_MONOTONIC, &tspec);
    return (tspec.tv_sec * 1000000000u + tspec.tv_nsec);
}

uint64_t MonoTime::MicroSeconds(void)
{
    struct timespec tspec;
    clock_gettime(CLOCK_MONOTONIC, &tspec);
    return (tspec.tv_sec * 1000000u + tspec.tv_nsec / 1000u);
}

uint64_t MonoTime::MilliSeconds(void)
{
    struct timespec tspec;
    clock_gettime(CLOCK_MONOTONIC, &tspec);
    return (tspec.tv_sec * 1000u + tspec.tv_nsec / 1000000u);
}

uint64_t MonoTime::Seconds(void)
{
    struct timespec tspec;
    clock_gettime(CLOCK_MONOTONIC, &tspec);
    return (tspec.tv_sec);
}

uint64_t RealTime::NanoSeconds(void)
{
    struct timespec tspec;
    clock_gettime(CLOCK_REALTIME, &tspec);
    return (tspec.tv_sec * 1000000000u + tspec.tv_nsec);
}

uint64_t RealTime::MicroSeconds(void)
{
    struct timespec tspec;
    clock_gettime(CLOCK_REALTIME, &tspec);
    return (tspec.tv_sec * 1000000u + tspec.tv_nsec / 1000u);
}

uint64_t RealTime::MilliSeconds(void)
{
    struct timespec tspec;
    clock_gettime(CLOCK_REALTIME, &tspec);
    return (tspec.tv_sec * 1000u + tspec.tv_nsec / 1000000u);
}

uint64_t RealTime::Seconds(void)
{
    struct timespec tspec;
    clock_gettime(CLOCK_REALTIME, &tspec);
    return (tspec.tv_sec);
}

size_t RealTime::Localtime(uint64_t stamp, const char* format, char* buf, size_t len)
{
    struct tm tmbuf;
    time_t val = static_cast<time_t>(stamp);
    return strftime(buf, len, format, localtime_r(&val, &tmbuf));
}

size_t RealTime::Gmtime(uint64_t stamp, const char* format, char* buf, size_t len)
{
    struct tm tmbuf;
    time_t val = static_cast<time_t>(stamp);
    return strftime(buf, len, format, gmtime_r(&val, &tmbuf));
}

size_t RealTime::Localtime(const char* format, char* buf, size_t len)
{
    struct tm tmbuf;
    time_t now = time(0);
    return strftime(buf, len, format, localtime_r(&now, &tmbuf));
}

size_t RealTime::Gmtime(const char* format, char* buf, size_t len)
{
    struct tm tmbuf;
    time_t now = time(0);
    return strftime(buf, len, format, gmtime_r(&now, &tmbuf));
}
