#ifndef IMESSAGESERVER_H
#define IMESSAGESERVER_H

/**
 * @file IMessageServer.h
 * @brief IMessageServer abstract interface.
 * @author Guillermo M. Paris
 * @date 2020-01-08
 */

#include "KeyFrequency.h"

class IMessageServer
{
public:
    virtual void     getTopHotkeys(KeyFrequencyVector&) = 0;
    virtual unsigned getTotalKeys() = 0;
    virtual bool     isHotKey(const std::string& key) = 0;
    virtual void     keySent(const std::string& key,
                             const std::string& url, unsigned port) = 0;
    virtual void     setTopKeyNumber(unsigned n) = 0;
    virtual bool     start() = 0;
    virtual bool     stop() = 0;
};


#endif // IMESSAGESERVER_H
