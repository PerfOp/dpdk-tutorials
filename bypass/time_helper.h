#ifndef TIME_HELPER_H
#define TIME_HELPER_H
#include <string>

struct MonoTime
{
    //
    static uint64_t NanoSeconds(void);

    //
    static uint64_t MicroSeconds(void);

    //
    static uint64_t MilliSeconds(void);

    //
    static uint64_t Seconds(void);
};

/*! RealTime
 */

struct RealTime
{
    //
    static uint64_t NanoSeconds(void);

    //
    static uint64_t MicroSeconds(void);

    //
    static uint64_t MilliSeconds(void);

    //
    static uint64_t Seconds(void);

    //
    static size_t Localtime(uint64_t stamp, const char* format, char* buf, size_t len);

    //
    static size_t Gmtime(uint64_t stamp, const char* format, char* buf, size_t len);

    //
    static size_t Localtime(const char* format, char* buf, size_t len);

    //
    static size_t Gmtime(const char* format, char* buf, size_t len);

    //
    static size_t Localtime(char* buf, size_t len)
    {
        return Localtime("%Y-%m-%d %H:%M:%S", buf, len);
    }

    //
    static size_t Gmtime(char* buf, size_t len) { return Gmtime("%Y-%m-%d %H:%M:%S", buf, len); }

    //
    static std::string Localtime(void)
    {
        char str[32];
        Localtime(str, sizeof(str));
        return std::string(str);
    }

    //
    static std::string Gmtime(void)
    {
        char str[32];
        Gmtime(str, sizeof(str));
        return std::string(str);
    }

    //
    static size_t Localtime(uint64_t stamp, char* buf, size_t len)
    {
        return Localtime(stamp, "%Y-%m-%d %H:%M:%S", buf, len);
    }

    //
    static size_t Gmtime(uint64_t stamp, char* buf, size_t len)
    {
        return Gmtime(stamp, "%Y-%m-%d %H:%M:%S", buf, len);
    }

    //
    static std::string Localtime(uint64_t stamp)
    {
        char str[32];
        Localtime(stamp, str, sizeof(str));
        return std::string(str);
    }

    //
    static std::string Gmtime(uint64_t stamp)
    {
        char str[32];
        Gmtime(stamp, str, sizeof(str));
        return std::string(str);
    }
};

class ElapsedTime
{
public:
    ElapsedTime(void) : m_stamp(MonoTime::NanoSeconds()) {}

    //
    uint64_t nanoSeconds(void) const { return (MonoTime::NanoSeconds() - m_stamp); }

    //
    uint64_t microSeconds(void) const { return (this->nanoSeconds() / 1000u); }

    //
    uint64_t milliSeconds(void) const { return (this->nanoSeconds() / 1000000u); }

    //
    uint64_t seconds(void) const { return (this->nanoSeconds() / 1000000000u); }

    //
    void reset(void) { m_stamp = MonoTime::NanoSeconds(); }

private:
    uint64_t m_stamp;
};

#endif //TIME_HELPER_H
