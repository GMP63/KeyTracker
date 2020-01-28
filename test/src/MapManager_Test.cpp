/**
 * @file MapManager_Test.cpp
 * @brief Unit tests for MapManager.
 * @author Guillermo M. Paris
 * @date 2019-12-15
 */

#include <cassert>
#include <ctime>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include "test-macros.h"
#include "test.h"
#include "KeyFrequency.h"
#include "MapManager.h"
#include "ThreadedMessageQueue.h"

extern const long CLOCKS_PER_MILLISEC;
extern const size_t maxNumbers;

/*****************************
 *    MapManager Tests       *
*****************************/

void mapmanager_massiveDirectKeyInsertion(MapManager& mgr)
{
    bool direct = true;
    keyInsertion(&mgr, nullptr, direct, maxNumbers/2); // 40 MKeys
}

void mapmanager_backupTest(MapManager& mgr)
{
    std::cout << "Performing data backup ...." << '\n';
    clock_t finish = 0, start = clock();
    mgr.backupRequest("Keys.csv", "Frequencies.csv");
    finish = clock();
    std::cout << "Backup done. It took "
              << ((finish - start) / CLOCKS_PER_MILLISEC)
              << " mSec\n" << std::endl;
}

size_t mapmanager_rankingTest(MapManager& mgr)
{
    KeyFrequencyVector vecResults;
    mgr.getTopHotkeys(vecResults);

    std::cout << "Most Frequent Keys" << '\n'
              << "==================" << "\n"
              << "Key            , Frequency\n";

    for(int i = 0; i < vecResults.size(); i++)
    {
        std::cout << vecResults[i].key << ", " << vecResults[i].frequency << '\n';
    }
    std::cout << "\nTotal non repeated keys: " << mgr.getTotalKeyNumber() << "\n\n";

    return vecResults.size();
}

void mapmanager_fullSequentialTest(TEST_REF, MapManager& mgr)
{
    mapmanager_massiveDirectKeyInsertion(mgr);
    EXPECT_LE(mgr.getTotalKeyNumber(), 40000);

    mapmanager_backupTest(mgr);

    size_t rankingSize = mapmanager_rankingTest(mgr);
    EXPECT_EQ(rankingSize, mgr.getTopKeyReportBaseSize());
    EXPECT_EQ(mgr.getTopKeyReportMaxSize(), 20);
}

void mapmanager_concurrencyTest(MapManager& mgr, bool directWriteTest)
{
    MapManager& m = mgr;
    auto searchWords =  [] (MapManager& mgr, const char* k) {
        clock_t start = clock();
        bool b = mgr.isHotKey(k);
        clock_t finish = clock();
        if (!b) {
            std::cout << "Searching took " << (finish - start)
                      << " uSec.\nAnd, obviously the key " << '"' << k << '"'
                      << " was not found! :=)" << std::endl;
        }
    };

    std::thread wt1;
    if (directWriteTest) wt1 = std::thread(mapmanager_massiveDirectKeyInsertion, std::ref(mgr));

    std::this_thread::sleep_for(std::chrono::seconds(directWriteTest ? 4 : 12));
    std::thread rt1(mapmanager_rankingTest, std::ref(mgr));
    std::this_thread::sleep_for(std::chrono::seconds(directWriteTest ? 2 : 5));
    std::thread rt2(searchWords, std::ref(mgr), "letter in the middle");
    std::thread rt3(searchWords, std::ref(mgr), "impossible to be found");
    std::thread rt4(mapmanager_rankingTest, std::ref(mgr));
    std::thread rt5(searchWords, std::ref(mgr), "any key you can put here");
    std::thread rt6(mapmanager_backupTest, std::ref(mgr));
    rt1.join();
    rt2.join();
    rt3.join();
    rt4.join();
    rt5.join();
    rt6.join();

    if (directWriteTest) wt1.join();
}

void mapmanager_functionalTest(TEST_REF)
{
    MapManager m(12, 24);
    EXPECT_Z(m.getTopKeyReportActualSize());
    EXPECT_EQ(m.getTopKeyReportBaseSize(), 12);
    EXPECT_EQ(m.getTopKeyReportMaxSize(), 24);
    m.addOrUpdateKey("AAAAAAAAAA", "aaa.com", 1);
    m.addOrUpdateKey("BBBBBBBBBB", "bbb.com", 2);
    m.addOrUpdateKey("AAAAAAAAAA", "aaa.com", 11);
    m.addOrUpdateKey("CCCCCCCCCC", "ccc.com", 3);
    m.addOrUpdateKey("AAAAAAAAAA", "aaa.com", 111);
    m.addOrUpdateKey("DDDDDDDDDD", "ddd.com", 4);
    m.addOrUpdateKey("AAAAAAAAAA", "aaa.com", 1111);
    m.addOrUpdateKey("EEEEEEEEEE", "eee.com", 5);
    m.addOrUpdateKey("AAAAAAAAAA", "aaa.com", 11111);
    m.addOrUpdateKey("FFFFFFFFFF", "fff.com", 6);
    m.addOrUpdateKey("AAAAAAAAAA", "aaa.com", 111111);
    m.addOrUpdateKey("GGGGGGGGGG", "ggg.com", 7);
    m.addOrUpdateKey("AAAAAAAAAA", "aaa.com", 1111111);
    m.addOrUpdateKey("HHHHHHHHHH", "hhh.com", 8);
    m.addOrUpdateKey("AAAAAAAAAA", "aaa.com", 11111111);
    m.addOrUpdateKey("IIIIIIIIII", "iii.com", 9);
    m.addOrUpdateKey("AAAAAAAAAA", "aaa.com", 111111111);
    m.addOrUpdateKey("JJJJJJJJJJ", "jjj.com", 10);
    m.addOrUpdateKey("BBBBBBBBBB", "bbb.com", 22);
    m.addOrUpdateKey("KKKKKKKKKK", "kkk.com", 11);
    m.addOrUpdateKey("BBBBBBBBBB", "bbb.com", 222);
    m.addOrUpdateKey("LLLLLLLLLL", "lll.com", 12);
    m.addOrUpdateKey("BBBBBBBBBB", "bbb.com", 2222);
    m.addOrUpdateKey("MMMMMMMMMM", "mmm.com", 13);
    m.addOrUpdateKey("BBBBBBBBBB", "bbb.com", 22222);
    m.addOrUpdateKey("NNNNNNNNNN", "nnn.com", 14);
    m.addOrUpdateKey("BBBBBBBBBB", "bbb.com", 222222);
    m.addOrUpdateKey("OOOOOOOOOO", "ooo.com", 15);
    m.addOrUpdateKey("BBBBBBBBBB", "bbb.com", 2222222);
    m.addOrUpdateKey("PPPPPPPPPP", "ppp.com", 16);
    m.addOrUpdateKey("BBBBBBBBBB", "bbb.com", 22222222);
    m.addOrUpdateKey("QQQQQQQQQQ", "qqq.com", 17);
    m.addOrUpdateKey("CCCCCCCCCC", "ccc.com", 33);
    m.addOrUpdateKey("RRRRRRRRRR", "rrr.com", 18);
    m.addOrUpdateKey("CCCCCCCCCC", "ccc.com", 333);
    m.addOrUpdateKey("SSSSSSSSSS", "sss.com", 19);
    m.addOrUpdateKey("CCCCCCCCCC", "ccc.com", 3333);
    m.addOrUpdateKey("TTTTTTTTTT", "ttt.com", 20);
    m.addOrUpdateKey("CCCCCCCCCC", "ccc.com", 33333);
    m.addOrUpdateKey("UUUUUUUUUU", "uuu.com", 21);
    m.addOrUpdateKey("CCCCCCCCCC", "ccc.com", 333333);
    m.addOrUpdateKey("VVVVVVVVVV", "vvv.com", 22);
    m.addOrUpdateKey("DDDDDDDDDD", "ddd.com", 44);
    m.addOrUpdateKey("WWWWWWWWWW", "www.com", 23);
    m.addOrUpdateKey("DDDDDDDDDD", "ddd.com", 444);
    m.addOrUpdateKey("XXXXXXXXXX", "xxx.com", 24);
    m.addOrUpdateKey("DDDDDDDDDD", "ddd.com", 4444);
    m.addOrUpdateKey("YYYYYYYYYY", "yyy.com", 25);
    m.addOrUpdateKey("ZZZZZZZZZZ", "zzz.com", 26);
    EXPECT_TRUE(m.isHotKey("AAAAAAAAAA"));
    EXPECT_TRUE(m.isHotKey("BBBBBBBBBB"));
    EXPECT_TRUE(m.isHotKey("CCCCCCCCCC"));
    EXPECT_TRUE(m.isHotKey("DDDDDDDDDD"));
    EXPECT_EQ(m.getTotalKeyNumber(), 26);

    KeyFrequencyVector v;

    m.getTopHotkeys(v);
    EXPECT_EQ(m.getTopKeyReportActualSize(), 24);
    EXPECT_EQ(v.size(), 12);
    EXPECT_EQ(v[0].key, std::string("AAAAAAAAAA"));
    EXPECT_EQ(v[1].key, std::string("BBBBBBBBBB"));
    EXPECT_EQ(v[2].key, std::string("CCCCCCCCCC"));
    EXPECT_EQ(v[3].key, std::string("DDDDDDDDDD"));

    EXPECT_EQ(std::stoi(v[0].frequency), 9);
    EXPECT_EQ(std::stoi(v[1].frequency), 8);
    EXPECT_EQ(std::stoi(v[2].frequency), 6);
    EXPECT_EQ(std::stoi(v[3].frequency), 4);
}

void MapManagerTests(TEST_REF, MapManager& mm)
{
    // This test assumes uses their own freshly created map manager.
    // It does not interfere will all other sequential tests.
    mapmanager_functionalTest(TEST);

    std::cout << "\nLimited word set test starting ...\n";
    mapmanager_fullSequentialTest(TEST, mm);
    std::cout << "Limited word set test finished.\n";
    std::cout << "\nRead/Write concurrency test starting ...\n";
    bool directWriteTest = true;
    mapmanager_concurrencyTest(mm, directWriteTest);
    std::cout << "Read/Write concurrency test finished.\n" << std::endl;
}
