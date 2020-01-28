#ifndef IMAPMANAGER_H
#define IMAPMANAGER_H

/**
 * @file MapManager.h
 * @brief IMapManager abstract interface.
 * @author Guillermo M. Paris
 * @date 2019-12-15
 */

#include "KeyFrequency.h"

class IMapManager
{
public:
    virtual void      addOrUpdateKey(const std::string& key, const std::string& url, unsigned int port) = 0;
    virtual void      setTopKeyReportBaseSize(unsigned short baseSize) = 0;
    virtual unsigned  getTopKeyReportBaseSize() = 0;
    virtual unsigned  getTopKeyReportActualSize() = 0;
    virtual unsigned  getTopKeyReportMaxSize() = 0;
    virtual unsigned  getTotalKeyNumber() = 0;
    virtual void      getTopHotkeys(KeyFrequencyVector&) = 0;
    virtual bool      isHotKey(const std::string& key) = 0;
    virtual bool      backupRequest(const std::string& keyFile, const std::string& freqFile) = 0;
    virtual bool      restoreRequest(const std::string& keyFile, const std::string& freqFile) = 0;
};

#endif // IMAPMANAGER_H
