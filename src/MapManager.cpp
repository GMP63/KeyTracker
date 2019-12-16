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
        pKeyMap(new strdetailsMap), pFrecuencyMap(new ulongstrMap), writingData(0)
{
}

MapManager::MapManager(unsigned int n, unsigned int m /* = 0 */)
    :   baseReportElementNumber(n), maxReportElementNumber(m > n + 4 ? m : n + 4),
        pKeyMap(new strdetailsMap), pFrecuencyMap(new ulongstrMap), writingData(0)
{
}

MapManager::~MapManager()
{
    if (pKeyMap)       delete pKeyMap;
    if (pFrecuencyMap) delete pFrecuencyMap;
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
        if(itKeys->first.compare(key) == 0) // key found! Just increment the frecuency count
        {
            prevOrderNo = itKeys->second.frecuency;
//          std::cerr << "debug. Found Key " << key << "  " << prevOrderNo << " times.\n";
            orderNo = 1 + prevOrderNo;
        }
    }
    // search shortened by hint (itKeys).
    pKeyMap->insert_or_assign( itKeys, std::string(key), KeyDetails(url, nPort, orderNo) );

    unsigned long lowestInTopRank = (pFrecuencyMap->empty() ? 1L : pFrecuencyMap->begin()->first);
    unsigned int actualSize = pFrecuencyMap->size();
    if (orderNo <= lowestInTopRank && actualSize >= maxReportElementNumber)
    {   // It is really not worth insertion but then later deletion by size maintenance.
        writingData = 0;
        mapCondition.notify_one();
        return;
    }

    // Search, delete and insert on the frecuency map.

    if (prevOrderNo > 0) // if this key is already present in the ranking, the old record must be removed
    {
        ulongstrIterator itRanking = pFrecuencyMap->find(prevOrderNo);
        ulongstrIterator itRankingEnd = pFrecuencyMap->end();

        while ( itRanking != itRankingEnd &&  // frecuency number found in frecuency table and ...
                itRanking->second != key && itRanking->first == prevOrderNo )
        {
            ++itRanking;
        }

        if (itRanking != itRankingEnd && itRanking->first == prevOrderNo)
        {
            pFrecuencyMap->erase(itRanking);  // combination {prevOrderNo, key} was really found.
//          std::cerr << "debug. Frecuency " << prevOrderNo << ", Key: " << key << ", eliminated from the ranking.\n";
        }

    }

    pFrecuencyMap->insert({orderNo, std::string(key)});

    while (pFrecuencyMap->size() > maxReportElementNumber)
    {                                      // Size maintainance that keep reduced and constant the size of this map.
        ulongstrIterator itRanking = pFrecuencyMap->begin();
        pFrecuencyMap->erase(itRanking++);
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

    unsigned long orderNo = itKeys->second.frecuency;
    unsigned long lowestInTopRank = (pFrecuencyMap->empty() ? 0L : pFrecuencyMap->begin()->first);
    if (orderNo < lowestInTopRank)  return false; // orderNo would never be found, no worth searching for it.

    ulongstrIterator itRankingEnd = pFrecuencyMap->end();
    ulongstrIterator itRanking = pFrecuencyMap->find(orderNo);
    if (itRanking == itRankingEnd) return false; // orderNo not found. Should never happen!

    while ( itRanking != itRankingEnd && itRanking->second != key
            && itRanking->first == orderNo )
    {
        ++itRanking;
    }

    return itRanking != itRankingEnd && itRanking->first == orderNo;
}

void MapManager::getTopHotkeys(KeyFrecuencyVector& vec)
{
    unsigned int index = 0;

    std::unique_lock<std::mutex> lock(mapMutex);
    mapCondition.wait( lock, [this]{return this->writingData == 0;} );

    ulongstrMap::reverse_iterator itRevRanking = pFrecuencyMap->rbegin();
    ulongstrMap::reverse_iterator itRevRankingEnd = pFrecuencyMap->rend();
    while (itRevRanking != itRevRankingEnd && index < baseReportElementNumber)
    {
        vec.push_back( KeyFrecuency( // move to record, not a copy
                          std::string(itRevRanking->second), std::to_string(itRevRanking->first) ));
        ++itRevRanking;
        ++index;
    }
}

bool MapManager::backupRequest(const std::string& keyFilename, const std::string& frecFilename)
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

    std::ofstream  outFrecFile(frecFilename);
    if (outFrecFile.bad())
    {
        logError(frecFilename, "opening");
        outKeyFile.close();
        outFrecFile.close();
        return false;
    }

    std::unique_lock<std::mutex>  lock(mapMutex);
    mapCondition.wait( lock, [this]{return this->writingData == 0;} );

    // Iterate and dump the frecuency map.

    ulongstrIterator itRanking = pFrecuencyMap->begin();
    ulongstrIterator itRankingEnd = pFrecuencyMap->end();
    while (itRanking != itRankingEnd)
    {
        outFrecFile << itRanking->first << fieldSeparator << itRanking->second << '\n';

        if (outFrecFile.bad())
        {
            logError(frecFilename, "writing");
            return false;
        }

        ++itRanking;
    }

    outFrecFile.close();

    // Iterate and dump the key map.
    strdetailsIterator itKeys = pKeyMap->begin();
    strdetailsIterator itKeyEnd = pKeyMap->end();
    while (itKeys != itKeyEnd)
    {
        outKeyFile << itKeys->first << fieldSeparator << itKeys->second.frecuency
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

bool MapManager::restoreRequest(const std::string& keyFilename, const std::string& frecFilename)
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

    std::ifstream  inFrecFile(frecFilename);
    if (inFrecFile.bad())
    {
        logError(frecFilename, "opening");
        inKeyFile.close();
        inFrecFile.close();
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
            inFrecFile.close();
            return false;
        }

        pKeyMap->insert(pKeyMap->end(), {sKey, KeyDetails(sUrl, unsigned(std::stoi(sPort)), std::stoul(sOrderNum))});
    }
    inKeyFile.close();

    if (pKeyMap->size() > 0) pKeyMap->clear();
    while (!inFrecFile.eof())
    {
        std::getline(inFrecFile, sOrderNum, fieldSeparator);
        std::getline(inFrecFile, sKey);
        if (inFrecFile.bad())
        {
            logError(keyFilename, "reading");
            inKeyFile.close();
            inFrecFile.close();
            return false;
        }

        pFrecuencyMap->insert({std::stoul(sOrderNum), sKey});
    }

    writingData = 0;
    mapCondition.notify_one();
    inFrecFile.close();
    return true;
}

void MapManager::purge(unsigned short newFrecuencyMapLength) // purge n , N <= n <= M
{
    unsigned int i = 0, n = newFrecuencyMapLength;
    if (n > maxReportElementNumber)    n = maxReportElementNumber;
    else if (n < baseReportElementNumber) n = baseReportElementNumber;

    strdetailsMap* pNewkeyMap = new strdetailsMap;
    std::lock_guard<std::mutex> lock(mapMutex);
    writingData = 1;

    strdetailsIterator itKeys = pKeyMap->begin();
    strdetailsIterator itKeyEnd = pKeyMap->end();
    ulongstrMap::reverse_iterator itRevRanking = pFrecuencyMap->rbegin();
    ulongstrMap::reverse_iterator itRevRankingEnd = pFrecuencyMap->rend();
    while (itRevRanking != itRevRankingEnd && i < n)
    {
        std::string key(itRevRanking->second);
        itKeys = pKeyMap->find(key);
        if (itKeys == itKeyEnd) // should never happen
        {
            pFrecuencyMap->erase(--itRevRanking.base()); // erase the orphan key
            std::cerr << "WARNING: Erasing orphan key from the ranking: " << key << std::endl;
        }
        else
        {
            pNewkeyMap->insert(std::make_pair(key, itKeys->second));
        }

        ++i; ++itRevRanking;
    }

    if (itRevRanking != itRevRankingEnd) // There are still more elements in "the ranking table".
        pFrecuencyMap->erase(pFrecuencyMap->begin(), --itRevRanking.base()); // purge them

    if(pKeyMap) delete pKeyMap;
    pKeyMap = pNewkeyMap;

    writingData = 0;
    mapCondition.notify_one();
}

void MapManager::zap()
{
    std::lock_guard<std::mutex> lock(mapMutex);
    writingData = 1;

    pFrecuencyMap->clear();
    pKeyMap->clear();

    writingData = 0;
    mapCondition.notify_one();
}
