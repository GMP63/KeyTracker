/**
 * @file MapManager.cpp
 * @brief MapManager implementation.
 *        Mapmanager is the core component of key tracker. MapManager implementation.
 * @author Guillermo M. Paris
 * @date 2019-12-15
 */

#include <fstream>
#include <iostream>
#include "MapManager.h"


MapManager::MapManager()
    :   baseReportElementNumber(DEFAULTTopKey), maxReportElementNumber(MAXTopKey),
        pKeyMap(new strdetailsMap), pFrequencyMap(new ulongstrMap), writingData(0)
{
}

MapManager::MapManager(unsigned int n, unsigned int m /* = 0 */)
    :   baseReportElementNumber(n), maxReportElementNumber(m > n + 4 ? m : n + 4),
        pKeyMap(new strdetailsMap), pFrequencyMap(new ulongstrMap), writingData(0)
{
}

MapManager::~MapManager()
{
    if (pKeyMap)       delete pKeyMap;
    if (pFrequencyMap) delete pFrequencyMap;
}

void MapManager::addOrUpdateKey(const std::string& key, const std::string& url, unsigned int nPort)
{
    unsigned long prevOrderNo = 0L, orderNo = 1L;

    std::lock_guard<std::mutex> lock(mapMutex);
    writingData = 1;

    // Search, insert or update on the key map.
    strdetailsIterator itKeyEnd = pKeyMap->end();
    strdetailsIterator itKeys = pKeyMap->lower_bound(key); // not parallel

    if (itKeys != itKeyEnd)
    {
        if(itKeys->first.compare(key) == 0) // key found! Just increment the frequency count
        {
            prevOrderNo = itKeys->second.frequency;
//          std::cerr << "debug. Found Key " << key << "  " << prevOrderNo << " times.\n";
            orderNo = 1 + prevOrderNo;
        }
    }
    // search shortened by hint (itKeys).
    pKeyMap->insert_or_assign( itKeys, std::string(key), KeyDetails(url, nPort, orderNo) );

    unsigned long lowestInTopRank = (pFrequencyMap->empty() ? 1L : pFrequencyMap->begin()->first);
    unsigned int actualSize = pFrequencyMap->size();
    if (orderNo <= lowestInTopRank && actualSize >= maxReportElementNumber)
    {   // It is really not worth insertion but then later deletion by size maintenance.
        writingData = 0;
        mapCondition.notify_one();
        return;
    }

    // Search, delete and insert on the frequency map.

    if (prevOrderNo > 0) // if this key is already present in the ranking, the old record must be removed
    {
        ulongstrIterator itRanking = pFrequencyMap->find(prevOrderNo);
        ulongstrIterator itRankingEnd = pFrequencyMap->end();

        while ( itRanking != itRankingEnd &&  // frequency number found in frequency table and ...
                itRanking->second != key && itRanking->first == prevOrderNo )
        {
            ++itRanking;
        }

        if (itRanking != itRankingEnd && itRanking->first == prevOrderNo)
        {
            pFrequencyMap->erase(itRanking);  // combination {prevOrderNo, key} was really found.
//          std::cerr << "debug. Frequency " << prevOrderNo << ", Key: " << key << ", eliminated from the ranking.\n";
        }

    }

    pFrequencyMap->insert({orderNo, std::string(key)});

    while (pFrequencyMap->size() > maxReportElementNumber)
    {                                      // Size maintainance that keep reduced and constant the size of this map.
        ulongstrIterator itRanking = pFrequencyMap->begin();
        pFrequencyMap->erase(itRanking++);
    }

    writingData = 0;
    mapCondition.notify_one();
}

void MapManager::setTopKeyReportBaseSize(unsigned short newBase)
{
    std::lock_guard<std::mutex> lock(mapMutex);
    writingData = 1;

    if (newBase > maxReportElementNumber)
    {
        baseReportElementNumber = maxReportElementNumber;
    }
    else
    {
        baseReportElementNumber = newBase;
    }

    writingData = 0;
}

bool MapManager::isHotKey(const std::string& key)
{
    std::unique_lock<std::mutex>  lock(mapMutex);
    mapCondition.wait( lock, [this]{return this->writingData == 0;} );

    strdetailsIterator itKeyEnd = pKeyMap->end();
    strdetailsIterator itKeys = pKeyMap->find(key);
    if (itKeys == itKeyEnd) return false; // supplied key not found in keyMap

    unsigned long orderNo = itKeys->second.frequency;
    unsigned long lowestInTopRank = (pFrequencyMap->empty() ? 0L : pFrequencyMap->begin()->first);
    if (orderNo < lowestInTopRank)  return false; // orderNo would never be found, no worth searching for it.

    ulongstrIterator itRankingEnd = pFrequencyMap->end();
    ulongstrIterator itRanking = pFrequencyMap->find(orderNo);
    if (itRanking == itRankingEnd) return false; // orderNo not found. Should never happen!

    while ( itRanking != itRankingEnd && itRanking->second != key
            && itRanking->first == orderNo )
    {
        ++itRanking;
    }

    return itRanking != itRankingEnd && itRanking->first == orderNo;
}

void MapManager::getTopHotkeys(KeyFrequencyVector& vec)
{
    unsigned int index = 0;

    std::unique_lock<std::mutex> lock(mapMutex);
    mapCondition.wait( lock, [this]{return this->writingData == 0;} );

    ulongstrMap::reverse_iterator itRevRanking = pFrequencyMap->rbegin();
    ulongstrMap::reverse_iterator itRevRankingEnd = pFrequencyMap->rend();
    while (itRevRanking != itRevRankingEnd && index < baseReportElementNumber)
    {
        vec.push_back( KeyFrequency( // move to record, not a copy
                          std::string(itRevRanking->second), std::to_string(itRevRanking->first) ));
        ++itRevRanking;
        ++index;
    }
}

bool MapManager::backupRequest(const std::string& keyFilename, const std::string& freqFilename)
{
    static const char* szMsg1 = "Error ";
    static const char* szMsg2 =  " output file: ";
    static const char* szMsg3 = " . Backup operation aborted.\n";
    auto logError = [&](const std::string& fname, const char* verb) {
        std::cerr << szMsg1 << verb << szMsg2 << fname << szMsg3 << std::endl;
    };

    std::ofstream  outKeyFile(keyFilename);
    if (outKeyFile.bad())
    {
        logError(keyFilename, "opening");
        outKeyFile.close();
        return false;
    }

    std::ofstream  outFreqFile(freqFilename);
    if (outFreqFile.bad())
    {
        logError(freqFilename, "opening");
        outKeyFile.close();
        outFreqFile.close();
        return false;
    }

    std::unique_lock<std::mutex>  lock(mapMutex);
    mapCondition.wait( lock, [this]{return this->writingData == 0;} );

    // Iterate and dump the frequency map.

    ulongstrIterator itRanking = pFrequencyMap->begin();
    ulongstrIterator itRankingEnd = pFrequencyMap->end();
    while (itRanking != itRankingEnd)
    {
        outFreqFile << itRanking->first << fieldSeparator << itRanking->second << '\n';

        if (outFreqFile.bad())
        {
            logError(freqFilename, "writing");
            return false;
        }

        ++itRanking;
    }

    outFreqFile.close();

    // Iterate and dump the key map.
    strdetailsIterator itKeys = pKeyMap->begin();
    strdetailsIterator itKeyEnd = pKeyMap->end();
    while (itKeys != itKeyEnd)
    {
        outKeyFile << itKeys->first << fieldSeparator << itKeys->second.frequency
        << fieldSeparator << itKeys->second.url << fieldSeparator << itKeys->second.port << '\n';

        ++itKeys;
    }
    if (outKeyFile.bad())
    {
        logError(keyFilename, "writing");
        return false;
    }

    outKeyFile.close();
    return true;
}

bool MapManager::restoreRequest(const std::string& keyFilename, const std::string& freqFilename)
{
    static const char* szMsg1 = "Error ";
    static const char* szMsg2 =  " input file: ";
    static const char* szMsg3 = " . Restore operation aborted.\n";
    auto logError = [&](const std::string& fname, const char* verb) {
        std::cerr << szMsg1 << verb << szMsg2 << fname << szMsg3 << std::endl;
    };

    std::ifstream  inKeyFile(keyFilename);
    if (inKeyFile.bad())
    {
        logError(keyFilename, "opening");
        inKeyFile.close();
        return false;
    }

    std::ifstream  inFreqFile(freqFilename);
    if (inFreqFile.bad())
    {
        logError(freqFilename, "opening");
        inKeyFile.close();
        inFreqFile.close();
        return false;
    }

    std::string sKey, sOrderNum, sUrl, sPort;

    std::unique_lock<std::mutex>  lock(mapMutex);
    writingData = 1;

    if (pKeyMap->size() > 0) pKeyMap->clear();
    while (!inKeyFile.eof())
    {
        std::getline(inKeyFile, sKey, fieldSeparator);
        std::getline(inKeyFile, sOrderNum, fieldSeparator);
        std::getline(inKeyFile, sUrl, fieldSeparator);
        std::getline(inKeyFile, sPort);
        if (inKeyFile.bad())
        {
            logError(keyFilename, "reading");
            inKeyFile.close();
            inFreqFile.close();
            return false;
        }

        pKeyMap->insert(pKeyMap->end(), {sKey, KeyDetails(sUrl, unsigned(std::stoi(sPort)), std::stoul(sOrderNum))});
    }
    inKeyFile.close();

    if (pKeyMap->size() > 0) pKeyMap->clear();
    while (!inFreqFile.eof())
    {
        std::getline(inFreqFile, sOrderNum, fieldSeparator);
        std::getline(inFreqFile, sKey);
        if (inFreqFile.bad())
        {
            logError(keyFilename, "reading");
            inKeyFile.close();
            inFreqFile.close();
            return false;
        }

        pFrequencyMap->insert({std::stoul(sOrderNum), sKey});
    }

    writingData = 0;
    mapCondition.notify_one();
    inFreqFile.close();
    return true;
}

void MapManager::purge(unsigned short newFrequencyMapLength) // purge n , N <= n <= M
{
    unsigned int i = 0, n = newFrequencyMapLength;
    if (n > maxReportElementNumber)    n = maxReportElementNumber;
    else if (n < baseReportElementNumber) n = baseReportElementNumber;

    strdetailsMap* pNewkeyMap = new strdetailsMap;
    std::lock_guard<std::mutex> lock(mapMutex);
    writingData = 1;

    strdetailsIterator itKeys = pKeyMap->begin();
    strdetailsIterator itKeyEnd = pKeyMap->end();
    ulongstrMap::reverse_iterator itRevRanking = pFrequencyMap->rbegin();
    ulongstrMap::reverse_iterator itRevRankingEnd = pFrequencyMap->rend();
    while (itRevRanking != itRevRankingEnd && i < n)
    {
        std::string key(itRevRanking->second);
        itKeys = pKeyMap->find(key);
        if (itKeys == itKeyEnd) // should never happen
        {
            pFrequencyMap->erase(--itRevRanking.base()); // erase the orphan key
            std::cerr << "WARNING: Erasing orphan key from the ranking: " << key << std::endl;
        }
        else
        {
            pNewkeyMap->insert(std::make_pair(key, itKeys->second));
        }

        ++i; ++itRevRanking;
    }

    if (itRevRanking != itRevRankingEnd) // There are still more elements in "the ranking table".
        pFrequencyMap->erase(pFrequencyMap->begin(), --itRevRanking.base()); // purge them

    if(pKeyMap) delete pKeyMap;
    pKeyMap = pNewkeyMap;

    writingData = 0;
    mapCondition.notify_one();
}

void MapManager::zap()
{
    std::lock_guard<std::mutex> lock(mapMutex);
    writingData = 1;

    pFrequencyMap->clear();
    pKeyMap->clear();

    writingData = 0;
    mapCondition.notify_one();
}
