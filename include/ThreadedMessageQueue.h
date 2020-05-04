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
#include "IMapManager.h"
#include "IQueue.h"

#define FIRST_POPING_LATENCY 1 // up to 1 mSec latency in the first pop()

using BLFMessageQueue = boost::lockfree::queue<Message*>;

class ThreadedMessageQueue : public IQueue<Message*, IMapManager*>, private BLFMessageQueue
{
public:
    static const int firstPopLatency; // = FIRST_POPING_LATENCY;

    ThreadedMessageQueue()
        : BLFMessageQueue(10), m_running(false), m_consumer(nullptr){}
    ThreadedMessageQueue(size_t initialSize)
        : BLFMessageQueue(initialSize), m_running(false), m_consumer(nullptr) {}

    ~ThreadedMessageQueue();

    virtual void setConsumer(IMapManager* pMgr) { m_consumer = pMgr; }
    virtual bool isEmpty() { return BLFMessageQueue::empty(); }
    virtual bool start();
    virtual bool stop();
    virtual void push(Message* pMsg);

private:
    void    run();

    bool          m_running;
    IMapManager*  m_consumer;
    std::thread   m_poper;
};

#endif // MESSAGETHREADEDQUEUE_H
