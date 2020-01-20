/**
 * @file ThreadedMessageQueue.cpp
 * @brief ThreadedMessageQueue implementation.
 *        A Queue + output thread for messaging to IMapManager derived objects.
 * @author Guillermo M. Paris
 * @date 2019-12-15
 */

#include <cassert>
#include <thread>
#include "IMapManager.h"
#include "ThreadedMessageQueue.h"

ThreadedMessageQueue::~ThreadedMessageQueue()
{
    while (!BLFMessageQueue::empty())
    {
        Message* pm = nullptr;
        if (pop(pm))
        {
            delete pm;
        }
    }
}

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
        Message* pm = nullptr;
        if (! pop(pm)) // queue is empty
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(firstPopLatency)); // 10 mSec
            continue;
        }

        switch (pm->getCommand())
        {
        case Message::Command::addKey:
            consumer->addOrUpdateKey(pm->getStringRef1(), pm->getStringRef2(), pm->getNumber());
            delete pm;
            break;

        case Message::Command::setRankingLength:
            consumer->setTopKeyReportBaseSize(pm->getNumber());
            delete pm;
            break;

        case Message::Command::stop:
            running = false;
            break;

        default:
            break;
        }
    }
}

void ThreadedMessageQueue::stop()
{
    running = false;
    poper.join();
}

void ThreadedMessageQueue::push(Message* pMsg)
{
    assert(pMsg != nullptr);

    if (running)
    {
        BLFMessageQueue::push(pMsg);
    }
}
