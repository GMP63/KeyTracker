#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <thread>

class Timer
{
public:
    Timer();
    Timer(unsigned int term, bool repeat = false);

    void setRepetitive(bool repeat);
    void setTimeout(unsigned int delay);
    void setInterval(unsigned int term);
    unsigned int getInterval() { return m_interval; }

    template<typename Function>
    bool start(Function&& function)
    {
        if (m_running || m_interval == 0)
            return false;

        m_running = true;

        std::thread t( [=]() {
            run(function);
        } );

        t.detach();
        return true;
    }

    void stop();


private:

    template<typename Function>
    void run(Function&& function)
    {
        do
        {
            if(!m_running) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(m_interval));
            if(!m_running) break;
            function();
        } while (m_periodic && m_running);

        m_running = false;
    }

    volatile bool m_periodic = false;
    volatile bool m_running = false;
    unsigned int m_interval = 0;
};

#endif // TIMER_H