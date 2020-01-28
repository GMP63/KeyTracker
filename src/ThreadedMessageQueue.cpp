/**
 * @file ThreadedMessageQueue.cpp
 * @brief ThreadedMessageQueue implementation.
 *        A Queue + output thread for messaging to IMapManager derived objects.
 * @author Guillermo M. Paris
 * @date 2019-12-15
 */

#include <cassert>
#include <thread>
#include "ThreadedMessageQueue.h"

const int ThreadedMessageQueue::firstPopLatency = FIRST_POPING_LATENCY;

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

bool ThreadedMessageQueue::start()
{
    if (m_consumer == nullptr) return false;

    m_running = true;
    m_poper = std::thread([this]{run();}); 
    return true;
}

void ThreadedMessageQueue::run()
{
    while (m_running)
    {
        if (empty())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(firstPopLatency)); // 1 mSec
            continue;
        }

        Message* pm = nullptr;
        if (! pop(pm)) // some unknown problem
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(firstPopLatency)); // 1 mSec
            continue;
        }

        switch (pm->getCommand())
        {
        case Message::Command::addKey:
            m_consumer->addOrUpdateKey(pm->getStringRef1(), pm->getStringRef2(), pm->getNumber());
            delete pm;
            break;

        case Message::Command::setRankingLength:
            m_consumer->setTopKeyReportBaseSize(pm->getShort());
            delete pm;
            break;

        case Message::Command::stop:
            m_running = false;
            break;

        default:
            break;
        }
    }
}

bool ThreadedMessageQueue::stop()
{
    m_running = false;
    m_poper.join();
    return true; // this implementation always return true.
}

void ThreadedMessageQueue::push(Message* pMsg)
{
    assert(pMsg != nullptr);

    if (m_running)
    {
        BLFMessageQueue::push(pMsg);
    }
}
