#ifndef IWEBSERVER_H
#define IWEBSERVER_H

/**
 * @file WebServer.h
 * @brief MessageServer interface.
 *        This is the source/destination for all the messages to/from HTTP socket.
 *        This implementation is around a generic web server agregation, but it can be easily
 *        adapted to work with any generic message source/destination.
 * @author Guillermo M. Paris
 * @date 2019-01-08
 */

#include "web/listener.h"
#include "icommunicationserver.h"

extern int main(int, char*[]);

class WebServer : public ICommunicationServer
{
public:

    using WebServerGetFunctor = std::function<int (const std::string&, std::string&)>;
    using WebServerPostFunctor = ICommunicationServer::IncommingMessageFunctor;

    WebServer() = delete;

    boost::asio::io_context & getExecutionContext()
        { return m_qListener->getExecutionContext(); }

    virtual void  setReporter(WebServerPostFunctor& pf);
    virtual bool  start();
    virtual bool  stop() { m_qListener->stop(); return true; }


private:

    friend int main(int, char*[]); // this object is a "light singleton".

    WebServer(boost::asio::ip::address ip, const unsigned short port,
              const unsigned short threads = 1, Verbosity v = 0);
//  ~WebServer();

    static int onGetMessage(const std::string& inTarget, std::string& outPayload);

    static WebServer*              ms_pInstance;

    const boost::asio::ip::address m_ipAddress;
    const unsigned short           m_nPort;
    const unsigned short           m_nThreads;
    // Verbosity or log level : 0:warnings & errors,  1:info,  2:trace,  3:debug
    const Verbosity                m_verbosity;
    WebServerGetFunctor            m_localFunctor;
    WebServerPostFunctor           m_topMessageFunctor;
    std::shared_ptr<Listener>      m_qListener;
};


#endif // IWEBSERVER_H
