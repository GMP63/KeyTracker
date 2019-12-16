/**
 * @file MapManager_Test.cpp
 * @brief Unit tests for ThreadedMessageQueue and MapManager.
 * @author Guillermo M. Paris
 * @date 2019-12-15
 */

#include <cassert>
#include <ctime>
#include <chrono>
#include <thread>
#include "test-macros.h"
#include "test.h"
#include "MapManager_Test.h"
#include "ThreadedMessageQueue.h"

/******************************
 * ThreadedMessageQueue Tests *
******************************/

void queue_KeyBurst(ThreadedMessageQueue& q)
{
    bool direct = false;
    keyInsertion(nullptr, &q, direct, maxNumbers/2); // 40 MKeys
}

void waitForQueueEmpty(ThreadedMessageQueue& q)
{
    std::cout << "Waiting for queue to be empty.\n";
    unsigned int i = 0;
    while (! q.empty())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        ++i;
        if (i % 5) // prints a dot every second.
        {
            std::cout << '.';
            std::cout.flush();
        }
    }

    std::cout << std::endl;
}

void MixManagerQueueTests(TEST_REF, MapManager& m, ThreadedMessageQueue& q)
{
    std::cout << "\nMessaging map manager through message queue. Test starting ...\n";
    queue_KeyBurst(q);
    std::cout << "Messaging map manager through message queue. Test finished.\n" << std::endl;
    waitForQueueEmpty(q);

    std::cout << "\nRead map manager, Write message queue and concurrency test starting ...\n";
    queue_KeyBurst(q);
    bool directWriteToManagerTest = false;
    mapmanager_concurrencyTest(m, directWriteToManagerTest);
    std::cout << "Read map manager, Write message queue and concurrency test finished.\n" << std::endl;
    waitForQueueEmpty(q);
}

