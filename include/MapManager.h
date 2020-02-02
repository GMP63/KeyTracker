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
#include "verbosity.h"

#define DEFAULT_TOPKEY_REPORTSIZE 10
#define MAX_TOPKEY_REPORTSIZE     20
#define MIN_GAP_MAX_DEFAULT        4
#define FIELD_SEPARATOR          ','

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
    MapManager(unsigned int n, unsigned int m = 0, unsigned int ll = 0);
    ~MapManager();

    virtual void       addOrUpdateKey(const std::string& key, const std::string& url, unsigned int port);
    virtual void       setTopKeyReportBaseSize(unsigned short base);
    virtual unsigned   getTopKeyReportBaseSize() {return m_baseReportElementNumber;}  // getN();
    virtual unsigned   getTopKeyReportActualSize() {return m_pFrequencyMap->size();}
    virtual unsigned   getTopKeyReportMaxSize() {return m_maxReportElementNumber;} // getM();
    virtual unsigned   getTotalKeyNumber() {return m_pKeyMap->size();}
    virtual void       getTopHotkeys(KeyFrequencyVector& vec);
    virtual bool       isHotKey(const std::string& key);
    virtual bool       backupRequest(const std::string& keyFilename, const std::string& freqFilename);
    virtual bool       restoreRequest(const std::string& keyFilename, const std::string& freqFilename);

    void               purge(unsigned short n); // purge n /  N <= n <= M
    void               zap();

private:
    static const char  ms_FieldSeparator = FIELD_SEPARATOR;
    static const short ms_DefaultTopKeySize = DEFAULT_TOPKEY_REPORTSIZE;
    static const short ms_MaxTopKeySize = MAX_TOPKEY_REPORTSIZE;

    // Verbosity or log level : 0:warnings & errors,  1:info,  2:trace,  3:debug
    const Verbosity    m_verbosity;

    unsigned short     m_baseReportElementNumber; // N = DEFAULT_TOPKEY_REPORTSIZE
    unsigned short     m_maxReportElementNumber;  // M = MAX_TOPKEY_REPORTSIZE
    strdetailsMap*     m_pKeyMap;
    ulongstrMap*       m_pFrequencyMap;
    int                m_writingData;
    std::mutex         m_mapMutex;
    std::condition_variable m_mapCondition;
};


#endif // MAPMANAGER_H