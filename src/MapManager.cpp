/**
 * @file MapManager.cpp
 * @brief MapManager implementation. Mapmanager is the core component of the Key Tracker.
 * @author Guillermo M. Paris
 * @date 2019-12-15
 */

#include <fstream>
#include <iostream>
#include "MapManager.h"

MapManager::MapManager()
    : m_verbosity(0)
    , m_baseReportElementNumber(ms_DefaultTopKeySize), m_maxReportElementNumber(ms_MaxTopKeySize),
      m_pKeyMap(new strdetailsMap), m_pFrequencyMap(new ulongstrMap), m_writingData(0)
{
}

MapManager::MapManager( unsigned int n,
                        unsigned int m /* = 0 */, unsigned int logLevel /* = 0 */)
    : m_verbosity(logLevel)
    , m_baseReportElementNumber(n)
    , m_maxReportElementNumber(m > n + MIN_GAP_MAX_DEFAULT ? m : n + MIN_GAP_MAX_DEFAULT)
    , m_pKeyMap(new strdetailsMap)
    , m_pFrequencyMap(new ulongstrMap)
    , m_writingData(0)
{
}

MapManager::~MapManager()
{
    if (m_pKeyMap)       delete m_pKeyMap;
    if (m_pFrequencyMap) delete m_pFrequencyMap;
}

void MapManager::addOrUpdateKey(const std::string& key, const std::string& url, unsigned int nPort)
{
    unsigned long prevOrderNo = 0L, orderNo = 1L;

    std::lock_guard<std::mutex> lock(m_mapMutex);
    m_writingData = 1;

    // Search, insert or update on the key map.
    strdetailsIterator itKeyEnd = m_pKeyMap->end();
    strdetailsIterator itKeys = m_pKeyMap->lower_bound(key); // not parallel

    if (itKeys != itKeyEnd)
    {
        if(itKeys->first.compare(key) == 0) // key found! Just increment the frequency count
        {
            prevOrderNo = itKeys->second.frequency;
            if (m_verbosity >= Verbosity::debug)
                std::cout << "Found Key " << key << "  " << prevOrderNo << " times.\n";

            orderNo = 1 + prevOrderNo;
        }
    }
    // search shortened by hint (itKeys).
    m_pKeyMap->insert_or_assign( itKeys, std::string(key), KeyDetails(url, nPort, orderNo) );

    unsigned long lowestInTopRank = (m_pFrequencyMap->empty() ? 1L : m_pFrequencyMap->begin()->first);
    unsigned int actualSize = m_pFrequencyMap->size();
    if (orderNo <= lowestInTopRank && actualSize >= m_maxReportElementNumber)
    {   // It is really not worth inserting but then later deleting by size maintenance.
        m_writingData = 0;
        m_mapCondition.notify_one();
        return;
    }

    // Search, delete and insert on the frequency map.

    if (prevOrderNo > 0) // if this key is already present in the ranking, the old record must be removed
    {
        ulongstrIterator itRanking = m_pFrequencyMap->find(prevOrderNo);
        ulongstrIterator itRankingEnd = m_pFrequencyMap->end();

        while ( itRanking != itRankingEnd &&  // frequency number found in frequency table and ...
                itRanking->second != key && itRanking->first == prevOrderNo )
        {
            ++itRanking;
        }

        if (itRanking != itRankingEnd && itRanking->first == prevOrderNo)
        {
            m_pFrequencyMap->erase(itRanking);  // combination {prevOrderNo, key} was really found.
            if (m_verbosity >= Verbosity::debug)
                std::cout << "Frequency " << prevOrderNo << ", Key: " << key << ", eliminated from the ranking.\n";
        }

    }

    m_pFrequencyMap->insert({orderNo, std::string(key)});

    while (m_pFrequencyMap->size() > m_maxReportElementNumber)
    {
        // Size maintainance that keeps this map's size constant.
        ulongstrIterator itRanking = m_pFrequencyMap->begin();
        m_pFrequencyMap->erase(itRanking++);
    }

    m_writingData = 0;
    m_mapCondition.notify_one();
}

void MapManager::setTopKeyReportBaseSize(unsigned short newBase)
{
    std::lock_guard<std::mutex> lock(m_mapMutex);
    m_writingData = 1;

    if (newBase > m_maxReportElementNumber)
    {
        m_baseReportElementNumber = m_maxReportElementNumber;
    }
    else
    {
        m_baseReportElementNumber = newBase;
    }

    m_writingData = 0;
}

bool MapManager::isHotKey(const std::string& key)
{
    std::unique_lock<std::mutex>  lock(m_mapMutex);
    m_mapCondition.wait( lock, [this]{return this->m_writingData == 0;} );

    strdetailsIterator itKeyEnd = m_pKeyMap->end();
    strdetailsIterator itKeys = m_pKeyMap->find(key);
    if (itKeys == itKeyEnd) return false; // supplied key not found in keyMap

    unsigned long orderNo = itKeys->second.frequency;
    unsigned long lowestInTopRank = (m_pFrequencyMap->empty() ? 0L : m_pFrequencyMap->begin()->first);
    if (orderNo < lowestInTopRank)  return false; // orderNo would never be found, no worth searching for it.

    ulongstrIterator itRankingEnd = m_pFrequencyMap->end();
    ulongstrIterator itRanking = m_pFrequencyMap->find(orderNo);
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

    std::unique_lock<std::mutex> lock(m_mapMutex);
    m_mapCondition.wait( lock, [this]{return this->m_writingData == 0;} );

    ulongstrMap::reverse_iterator itRevRanking = m_pFrequencyMap->rbegin();
    ulongstrMap::reverse_iterator itRevRankingEnd = m_pFrequencyMap->rend();
    while (itRevRanking != itRevRankingEnd && index < m_baseReportElementNumber)
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

    std::unique_lock<std::mutex>  lock(m_mapMutex);
    m_mapCondition.wait( lock, [this]{return this->m_writingData == 0;} );

    // Iterate and dump the frequency map.

    ulongstrIterator itRanking = m_pFrequencyMap->begin();
    ulongstrIterator itRankingEnd = m_pFrequencyMap->end();
    while (itRanking != itRankingEnd)
    {
        outFreqFile << itRanking->first << ms_FieldSeparator << itRanking->second << '\n';

        if (outFreqFile.bad())
        {
            logError(freqFilename, "writing");
            return false;
        }

        ++itRanking;
    }

    outFreqFile.close();

    // Iterate and dump the key map.
    strdetailsIterator itKeys = m_pKeyMap->begin();
    strdetailsIterator itKeyEnd = m_pKeyMap->end();
    while (itKeys != itKeyEnd)
    {
        outKeyFile << itKeys->first << ms_FieldSeparator << itKeys->second.frequency
        << ms_FieldSeparator << itKeys->second.url << ms_FieldSeparator << itKeys->second.port << '\n';

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
    if (inKeyFile.fail())
    {
        logError(keyFilename, "opening");
        inKeyFile.close();
        return false;
    }

    std::ifstream  inFreqFile(freqFilename);
    if (inFreqFile.fail())
    {
        logError(freqFilename, "opening");
        inKeyFile.close();
        inFreqFile.close();
        return false;
    }

    std::string sKey, sOrderNum, sUrl, sPort;

    std::unique_lock<std::mutex>  lock(m_mapMutex);
    m_writingData = 1;

    if (m_pKeyMap->size() > 0) m_pKeyMap->clear();
    while (!inKeyFile.eof())
    {
        std::getline(inKeyFile, sKey, ms_FieldSeparator);
        std::getline(inKeyFile, sOrderNum, ms_FieldSeparator);
        std::getline(inKeyFile, sUrl, ms_FieldSeparator);
        std::getline(inKeyFile, sPort);
        if (inKeyFile.eof())
            break;

        if (inKeyFile.fail())
        {
            logError(keyFilename, "reading");
            inKeyFile.close();
            inFreqFile.close();
            return false;
        }

        m_pKeyMap->insert( m_pKeyMap->end(),
                           {sKey, KeyDetails(sUrl, unsigned(std::stoi(sPort)), std::stoul(sOrderNum))} );
    }
    inKeyFile.close();

    if (m_pFrequencyMap->size() > 0) m_pFrequencyMap->clear();
    while (!inFreqFile.eof())
    {
        std::getline(inFreqFile, sOrderNum, ms_FieldSeparator);
        int no = inFreqFile.gcount();
        std::getline(inFreqFile, sKey);
        int nk = inFreqFile.gcount();
        if (inFreqFile.eof())
            break;

        if (inFreqFile.fail())
        {
            logError(keyFilename, "reading");
            inKeyFile.close();
            inFreqFile.close();
            return false;
        }

        m_pFrequencyMap->insert({std::stoul(sOrderNum), sKey});
    }

    m_writingData = 0;
    m_mapCondition.notify_one();
    inFreqFile.close();
    return true;
}

void MapManager::purge(unsigned short newFrequencyMapLength) // purge n , N <= n <= M
{
    unsigned int i = 0, n = newFrequencyMapLength;
    if (n > m_maxReportElementNumber)    n = m_maxReportElementNumber;
    else if (n < m_baseReportElementNumber) n = m_baseReportElementNumber;

    strdetailsMap* pNewkeyMap = new strdetailsMap;
    std::lock_guard<std::mutex> lock(m_mapMutex);
    m_writingData = 1;

    strdetailsIterator itKeys = m_pKeyMap->begin();
    strdetailsIterator itKeyEnd = m_pKeyMap->end();
    ulongstrMap::reverse_iterator itRevRanking = m_pFrequencyMap->rbegin();
    ulongstrMap::reverse_iterator itRevRankingEnd = m_pFrequencyMap->rend();
    while (itRevRanking != itRevRankingEnd && i < n)
    {
        std::string key(itRevRanking->second);
        itKeys = m_pKeyMap->find(key);
        if (itKeys == itKeyEnd) // should never happen
        {
            m_pFrequencyMap->erase(--itRevRanking.base()); // erase the orphan key
            std::cerr << "WARNING: Erasing orphan key from the ranking: " << key << std::endl;
        }
        else
        {
            pNewkeyMap->insert(std::make_pair(key, itKeys->second));
        }

        ++i; ++itRevRanking;
    }

    if (itRevRanking != itRevRankingEnd) // There are still more elements in "the ranking table".
        m_pFrequencyMap->erase(m_pFrequencyMap->begin(), --itRevRanking.base()); // purge them

    if(m_pKeyMap) delete m_pKeyMap;
    m_pKeyMap = pNewkeyMap;

    m_writingData = 0;
    m_mapCondition.notify_one();
}

void MapManager::zap()
{
    std::lock_guard<std::mutex> lock(m_mapMutex);
    m_writingData = 1;

    m_pFrequencyMap->clear();
    m_pKeyMap->clear();

    m_writingData = 0;
    m_mapCondition.notify_one();
}
