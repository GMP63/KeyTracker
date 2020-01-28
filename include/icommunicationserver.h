#ifndef ICOMMUNICATIONSERVER_H
#define ICOMMUNICATIONSERVER_H

/**
 * @file IMessageServer.h
 * @brief ICommunicationServer abstract interface.
 * @author Guillermo M. Paris
 * @date 2020-01-09
 */

#include <functional>

class ICommunicationServer
{
public:
    using IncommingMessageFunctor = std::function<int (const std::string&, const std::string&, std::string&)>;

    virtual void  setReporter(IncommingMessageFunctor&) = 0;
    virtual bool  start() = 0;
    virtual bool  stop() = 0;
};

#endif // ICOMMUNICATIONSERVER_H
