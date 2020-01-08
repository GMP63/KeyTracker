/**
 * @file test.cpp
 * @brief Unit tests file.
 * @author Guillermo M. Paris
 * @date 2019-12-10
 */

#include <cmath>
#include <ctime>
#include <iostream>
#include <random>
#include <thread>
#include "test-macros.h"
#include "MapManager_Test.h"
#include "ThreadedMessageQueue_Test.h"
#include "FirstHundredNumbersArray.h"

extern const long CLOCKS_PER_MILLISEC = CLOCKS_PER_SEC / 1000;
extern const size_t maxNumbers = 80000000;
const size_t initialQueueNodes = maxNumbers/8;


const char* urls[10] =
{
    "www.lacapital.com.ar", "www.lavoz.com.ar", "www.ellitoral.com.ar", "www.lanacion.com.ar","www.clarin.com",
    "www.ambito.com", "www.cronista.com", "www.infobae.com", "https://www.nytimes.com", "https://www.spiegel.de"
};


uint8_t genRandomNumbers[maxNumbers];

void populateArray()
{
    clock_t start = 0, finish = 0;
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<> d{50,15};

    std::cout << "Generating 80,000,000 random numbers ...." << '\n';
    start = clock();
    for(int i = 0; i < 80000000; i++)
    {
        genRandomNumbers[i] = static_cast<uint8_t>(std::round(d(gen)));
    }
    finish = clock();
    std::cout << "Done." << "\n";
    std::cout << "It took " << ((finish - start) / CLOCKS_PER_MILLISEC) << " mSec\n\n";
}

void keyInsertion(MapManager* pMgr, ThreadedMessageQueue* pMtq, bool direct, size_t burstSize)
{
    std::cout << "Inserting " << burstSize << " keys ...." << '\n';
    clock_t start = 0, finish = 0;
    std::string sKey;
    sKey.reserve(50);
    start = clock();
    for(int i = 0, j = 0; i < burstSize / 2; i++)
    {
        unsigned idx0 = genRandomNumbers[j++];
        unsigned idx1 = genRandomNumbers[j++];
        sKey = (firstHundred[idx0%100]);
        sKey += ' ';
        sKey += firstHundred[idx1%100];
        const char* pUrl = urls[(idx0 % 100)/10];
        unsigned int nPort = i + 1;

        if (direct && pMgr)
            pMgr->addOrUpdateKey(sKey, pUrl, nPort);
        else if (!direct && pMtq)
        {
            pMtq->push(new Message(Message::Command::addKey, sKey, pUrl, 0, 0, nPort));
        }
        else
        {
            std::cout << "BAD TEST PARAMETERS PASSED TO keyInsertion()" << '\n';
        }
    }

    finish = clock();
    std::cout << "Done." << "\n";
    std::cout << burstSize << " Keys " << (direct ? "direct insertion" : "queued")
              << " took " << ((finish - start) / CLOCKS_PER_MILLISEC) << " mSec\n"
              << std::endl;
}


int main(int argc, char* argv[])
{
    START_TESTS;

    // global common object instantiation here.
    populateArray();
    MapManager m;
    ThreadedMessageQueue q(initialQueueNodes);

    // test suites go here
    MapManagerTests(TEST, m);
    q.start(&m);
    MixManagerQueueTests(TEST, m, q);
    PRINT_RESULTS(std::cout);

    // global common object destruction here.
    q.stop();
}

