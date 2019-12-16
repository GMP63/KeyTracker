/**
 * @file ThreadedMessageQueue.cpp
 * @brief ThreadedMessageQueue implementation.
 *        A Queue + output thread for messaging to IMapManager derived objects.
 * @author Guillermo M. Paris
 * @date 2019-12-15
 */

#include <thread>
#include "IMapManager.h"
#include "ThreadedMessageQueue.h"

bool ThreadedMessageQueue::start(IMapManager* msgConsumer)
{
    if (msgConsumer == nullptr) return false;

    consumer = msgConsumer;
    running = true;
    poper = std::thread([this]{run();}); 
    return true;
}

void ThreadedMessageQueue::run()
{
    while (running)
    {
        Message m;
        pop(m);

        switch (m.getCommand())
        {
        case Message::Command::addKey:
            consumer->addOrUpdateKey(m.getStringRef1(), m.getStringRef2(), m.getNumber());
            break;

        case Message::Command::setRankingLength:
            consumer->setTopKeyReportBaseSize(m.getNumber());
            break;

        case Message::Command::stop:
            running = false;
            getWaitCondition().notify_all();
            break;

        default:
            break;
        }
    }
}

void ThreadedMessageQueue::stop()
{
    running = false;
    getWaitCondition().notify_all();
    poper.join();
}
