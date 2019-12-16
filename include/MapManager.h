#ifndef MAPMANAGER_H
#define MAPMANAGER_H

/**
 * @file MapManager.h
 * @brief MapManager interface.
 *        Mapmanager is the core component of key tracker. MapManager interface.
 * @author Guillermo M. Paris
 * @date 2019-12-15
 */

#include <condition_variable>
#include <map>
#include <mutex>
#include "IMapManager.h"

#define DEFAULT_TOPKEY_REPORTSIZE 10
#define MAX_TOPKEY_REPORTSIZE     20
#define FIELD_SEPARATOR           ','

class MapManager : public IMapManager
{
public:
    struct KeyDetails
    {
        KeyDetails(const std::string& s, unsigned int ui, unsigned long ul)
          : url(s), port(ui), frequency(ul) {}

        std::string   url;
        unsigned int  port;
        unsigned long frequency;
    };

    using strdetailsMap = std::map<std::string, KeyDetails>;
    using ulongstrMap = std::multimap<unsigned long, std::string>;
    using strdetailsIterator = std::map<std::string, KeyDetails>::iterator;
    using ulongstrIterator = std::multimap<unsigned long, std::string>::iterator;

    MapManager();
    MapManager(unsigned int n, unsigned int m = 0);
    ~MapManager();

    virtual void      addOrUpdateKey(const std::string& key, const std::string& url, unsigned int port);
    virtual void      setTopKeyReportBaseSize(unsigned short base);
    virtual unsigned  getTopKeyReportBaseSize() {return baseReportElementNumber;}  // getN();
    virtual unsigned  getTopKeyReportActualSize() {return pFrequencyMap->size();}
    virtual unsigned  getTopKeyReportMaxSize() {return maxReportElementNumber;} // getM();
    virtual unsigned  getTotalKeyNumber() {return pKeyMap->size();}
    virtual void      getTopHotkeys(KeyFrequencyVector& vec);
    virtual bool      isHotKey(const std::string& key);
    virtual bool      backupRequest(const std::string& keyFilename, const std::string& freqFilename);
    virtual bool      restoreRequest(const std::string& keyFilename, const std::string& freqFilename);

    void purge(unsigned short n); // purge n /  N <= n <= M
    void zap();

private:
    static const char  fieldSeparator = FIELD_SEPARATOR;
    static const short DEFAULTTopKey = DEFAULT_TOPKEY_REPORTSIZE;
    static const short MAXTopKey = MAX_TOPKEY_REPORTSIZE;

    unsigned short baseReportElementNumber; // N = DEFAULT_TOPKEY_REPORTSIZE
    unsigned short maxReportElementNumber;  // M = MAX_TOPKEY_REPORTSIZE
    strdetailsMap* pKeyMap;
    ulongstrMap*   pFrequencyMap;
    int            writingData;
    std::mutex     mapMutex;
    std::condition_variable mapCondition;
};


#endif // MAPMANAGER_H