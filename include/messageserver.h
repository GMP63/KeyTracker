#ifndef MESSAGESERVER_H
#define MESSAGESERVER_H

/**
 * @file MessageServer.h
 * @brief MessageServer interface.
 *        This is the source/destination for all the messages to/from MapManager object.
 *        This implementation is around a generic web server agregation, but it can be easily
 *        adapted to work with any generic message source/destination.
 * @author Guillermo M. Paris
 * @date 2019-01-08
 */

#include <functional>
#include "icommunicationserver.h"
#include "IMapManager.h"
#include "imessageserver.h"
#include "IQueue.h"
#include "Message.h"
#include "verbosity.h"

class BackupManager;

const int CMD_UNKNOWN   = -1; // Command not found.
const int CMD_FAILED    =  0; // Command execution failed.
const int CMD_SUCCEEDED =  1; // Command execution successfully completed.

class MessageServer : public IMessageServer
{
    static const char payloadDelimiter = ',';

public:

    using IMessageQueue = IQueue<Message*, IMapManager*>;
    using MessageServerFunctor = ICommunicationServer::IncommingMessageFunctor;

//  There won't be a functor map in this current implementation. There are few 'verb' strings.
//  using FunctorMap = std::map<std::string, MessageServerFunctor>;
//  using FunctorMapIterator = std::map<std::string, MessageServerFunctor>::iterator;

    MessageServer() = delete;

    virtual void     getTopHotkeys(KeyFrequencyVector& v)
                     { if (m_pMapManager) m_pMapManager->getTopHotkeys(v); }

    virtual unsigned getTotalKeys()
                     { return m_pMapManager ? m_pMapManager->getTotalKeyNumber() : 0; }

    virtual bool     isHotKey(const std::string& key)
                     { return m_pMapManager ? m_pMapManager->isHotKey(key) : false; }

    virtual void     keySent(const std::string& key, const std::string& url, unsigned int port)
                     { if (m_pMapManager) m_pMapManager->addOrUpdateKey(key, url, port); }

    virtual void     setTopKeyNumber(unsigned int n)
                     { if (m_pMapManager)  m_pMapManager->setTopKeyReportBaseSize(n); }

    virtual bool     start();
    virtual bool     stop();


private:

    friend int main(int, char*[]); // this object is a "light singleton".

    MessageServer( IMapManager* mgr, IMessageQueue* queue, BackupManager* bkp,
                   ICommunicationServer* comsrv, Verbosity v = 0);

    static int onMessage(const std::string& inTarget, const std::string& inData, std::string& outData);

    static MessageServer*   ms_pInstance;

    const Verbosity         m_verbosity;
    IMapManager*            m_pMapManager;
    IMessageQueue*          m_pMessageQueue;
    BackupManager*          m_pBackupManager;
    ICommunicationServer*   m_pCommServer;
    MessageServerFunctor    m_topMessageServerFunctor;
};


#endif // MESSAGESERVER_H
