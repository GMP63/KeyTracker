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
#include "Message.h"
#include "ThreadedQueue.h"

class IMapManager;

class ThreadedMessageQueue : public ThreadedQueue<Message>
{
public:
    ThreadedMessageQueue() { consumer = nullptr; }

    bool    start(IMapManager*);
    void    stop();

private:
    void    run();

    IMapManager*  consumer;
    std::thread   poper;
};

#endif // MESSAGETHREADEDQUEUE_H
