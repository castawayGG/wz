#include "timer.h"

namespace wz {

LARGE_INTEGER Timer::s_freq   = {};
bool          Timer::s_freqInit = false;

static void EnsureFreq() {
    if (!Timer::s_freqInit) {
        QueryPerformanceFrequency(&Timer::s_freq);
        Timer::s_freqInit = true;
    }
}

Timer::Timer() {
    EnsureFreq();
    Reset();
}

void Timer::Reset() {
    QueryPerformanceCounter(&m_start);
}

double Timer::ElapsedMs() const {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return static_cast<double>(now.QuadPart - m_start.QuadPart) * 1000.0
           / static_cast<double>(s_freq.QuadPart);
}

double Timer::ElapsedUs() const {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return static_cast<double>(now.QuadPart - m_start.QuadPart) * 1000000.0
           / static_cast<double>(s_freq.QuadPart);
}

void Timer::WaitUntilMs(double ms) const {
    while (ElapsedMs() < ms) {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        double remaining = ms - static_cast<double>(now.QuadPart - m_start.QuadPart)
                           * 1000.0 / static_cast<double>(s_freq.QuadPart);
        if (remaining > 2.0) Sleep(static_cast<DWORD>(remaining - 1.0));
        else YieldProcessor();
    }
}

double Timer::NowMs() {
    EnsureFreq();
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return static_cast<double>(t.QuadPart) * 1000.0
           / static_cast<double>(s_freq.QuadPart);
}

void Timer::SleepPreciseMs(double ms) {
    Timer t;
    t.WaitUntilMs(ms);
}

} // namespace wz
