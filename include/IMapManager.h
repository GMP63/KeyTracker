#ifndef IMAPMANAGER_H
#define IMAPMANAGER_H

/**
 * @file MapManager.h
 * @brief IMapManager interface.
 * @author Guillermo M. Paris
 * @date 2019-12-15
 */

#include <string>
#include <vector>

class IMapManager
{
public:
    struct KeyFrecuency
    {
        KeyFrecuency(const std::string&& sKey, const std::string&& sFrec)
          : key(sKey), frecuency(sFrec) {}
        ~KeyFrecuency() { key.clear(); frecuency.clear(); }

        std::string  key;
        std::string  frecuency;
    };

    using KeyFrecuencyVector = std::vector<KeyFrecuency>;

    virtual void      addOrUpdateKey(const std::string& key, const std::string& url, unsigned int port) = 0;
    virtual void      setTopKeyReportBaseSize(unsigned short baseSize) = 0;
    virtual unsigned  getTopKeyReportBaseSize() = 0;
    virtual unsigned  getTopKeyReportActualSize() = 0;
    virtual unsigned  getTopKeyReportMaxSize() = 0;
    virtual unsigned  getTotalKeyNumber() = 0;
    virtual void      getTopHotkeys(KeyFrecuencyVector&) = 0;
    virtual bool      isHotKey(const std::string& key) = 0;
    virtual bool      backupRequest(const std::string& keyFile, const std::string& frecFile) = 0;
    virtual bool      restoreRequest(const std::string& keyFile, const std::string& frecFile) = 0;
};

#endif // IMAPMANAGER_H
