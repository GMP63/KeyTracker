#ifndef MESSAGETHREADEDQUEUE_H
#define MESSAGETHREADEDQUEUE_H

/**
 * @file ThreadedMessageQueue.h
 * @brief ThreadedMessageQueue interface.
 *        A Queue + output thread for messaging to IMapManager derived objects.
 * @author Guillermo M. Paris
 * @date 2019-12-15
 */

#include <thread>
#include <boost/lockfree/queue.hpp>
#include "Message.h"

#define FIRST_POPING_LATENCY 10 // 10 mSec

using BLFMessageQueue = boost::lockfree::queue<Message*>;
class IMapManager;

class ThreadedMessageQueue : private BLFMessageQueue
{
public:
    static const int firstPopLatency = FIRST_POPING_LATENCY;

    ThreadedMessageQueue()
        : consumer(nullptr), running(false) {}
    ThreadedMessageQueue(size_t initialSize)
        : BLFMessageQueue(initialSize), consumer(nullptr), running(false) {}

    ~ThreadedMessageQueue();

    bool    isEmpty() { return BLFMessageQueue::empty(); }
    bool    start(IMapManager*);
    void    stop();
    void    push(Message* pMsg);

private:
    void    run();

    bool          running;
    IMapManager*  consumer;
    std::thread   poper;
};

#endif // MESSAGETHREADEDQUEUE_H
