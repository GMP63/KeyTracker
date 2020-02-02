
#include "timer/timer.h"

Timer::Timer()
    : m_periodic(false)
    , m_running(false)
    , m_interval(0)
{
}

Timer::Timer(unsigned int term, bool repeat /* = false */)
    : m_periodic(repeat)
    , m_running(false)
    , m_interval(term)
{
}

void Timer::setRepetitive(bool repeat)
{
    m_periodic = repeat;
}

void Timer::setTimeout(unsigned int delay)
{
    m_periodic = false;
    m_interval = delay;
}

void Timer::setInterval(unsigned int term)
{
    m_periodic = true;
    m_interval = term;
}

void Timer::stop()
{
    m_running = false;
}
