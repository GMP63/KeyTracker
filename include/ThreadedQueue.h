#ifndef THREADEDQUEUE_H
#define THREADEDQUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>

template<typename T>
class ThreadedQueue
{
public:
    ThreadedQueue() { running = false; }
    ThreadedQueue(ThreadedQueue const& other)
    {
        std::lock_guard<std::mutex> lock(other.queueMutex);
        running = false;
        queue = other.queue;
    }

    void enableExternalThreading()
    {
        running = true;
    }

    void disableExternalThreading()
    {
        running = false;
    }

    void push(T&& newValue)
    {
        if (!running) return;
        std::lock_guard<std::mutex> lock(queueMutex);
        if (!running) return;
        queue.push(std::move(newValue));
        queueCondition.notify_one();
    }

    void pop(T& value)
    {
        if (!running) return;
        std::unique_lock<std::mutex> lock(queueMutex);
        queueCondition.wait( lock, [this]{return !running || !queue.empty();} );
        if (!running) return;
        value = queue.front();
        queue.pop();
    }

    bool  empty() const
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        return queue.empty();
    }

    size_t  size() const
    {
       std::lock_guard<std::mutex> lock(queueMutex);
        return queue.size();
    }


protected:
    std::condition_variable& getWaitCondition()
    {
        return queueCondition;
    }

    bool                     running;


private:
    mutable std::mutex       queueMutex;
    std::condition_variable  queueCondition;
    std::queue<T>            queue;
};


#endif // THREADEDQUEUE_H