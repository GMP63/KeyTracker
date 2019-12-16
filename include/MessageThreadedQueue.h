#ifndef MESSAGETHREADEDQUEUE_H
#define MESSAGETHREADEDQUEUE_H

/**
 * @file MessageThreadedQueue.h
 * @brief MessageThreadedQueue interface.
 *        A Queue + output thread for messaging to IMapManager derived objects.
 * @author Guillermo M. Paris
 * @date 2019-12-15
 */

#include <thread>
#include "Message.h"
#include "ThreadedQueue.h"

class IMapManager;

class MessageThreadedQueue : public ThreadedQueue<Message>
{
public:
    MessageThreadedQueue() { consumer = nullptr; }

    bool    start(IMapManager*);
    void    stop();

private:
    void    run();

    IMapManager*  consumer;
    std::thread   poper;
};

#endif // MESSAGETHREADEDQUEUE_H
