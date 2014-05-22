#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <ostream>

class Timer
{
    typedef std::chrono::high_resolution_clock HighResolutionClock;
    typedef std::chrono::milliseconds Milliseconds;
public:
    explicit Timer(bool run = false)
    {
        if (run)
            reset();
    }

    void reset()
    {
        _start = HighResolutionClock::now();
    }

    Milliseconds elapsed() const
    {
        return std::chrono::duration_cast<Milliseconds>(HighResolutionClock::now() - _start);
    }

    template <typename T, typename Traits>
    friend std::basic_ostream<T, Traits>& operator<<(std::basic_ostream<T, Traits>& out, const Timer& timer)
    {
        return out << timer.elapsed().count();
    }

private:
    HighResolutionClock::time_point _start;
};

#endif // TIMER_H
