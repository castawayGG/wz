#pragma once
#include <Windows.h>
#include <cstdint>

namespace wz {

// High-resolution timer using QueryPerformanceCounter
class Timer {
public:
    Timer();

    // Reset the timer
    void Reset();

    // Elapsed time since last Reset() in milliseconds
    double ElapsedMs() const;

    // Elapsed time in microseconds
    double ElapsedUs() const;

    // Sleep until at least 'ms' milliseconds have elapsed since last Reset
    void WaitUntilMs(double ms) const;

    // Static helpers
    static double NowMs();
    static void SleepPreciseMs(double ms);

private:
    LARGE_INTEGER m_start;
    static LARGE_INTEGER s_freq;
    static bool          s_freqInit;
};

} // namespace wz
