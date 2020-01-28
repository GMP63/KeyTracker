/**
 * @file WebServer.cpp
 * @brief WebServer implementation.
 *        This is the source/destination for all the messages to/from HTTP socket.
 *        This implementation is around a generic web server agregation, but it can be easily
 *        adapted to work with any generic message source/destination.
 * @author Guillermo M. Paris
 * @date 2019-01-09
 */

#include <boost/beast/core.hpp>
#include <boost/asio.hpp>
#include <cassert>
#include <iostream>
#include <memory>

#include "web/listener.h"
#include "web/webserver.h"

namespace beast = boost::beast;  // from <boost/beast.hpp>
namespace net = boost::asio;     // from <boost/asio.hpp>

WebServer*  WebServer::ms_pInstance(nullptr);

WebServer::WebServer( net::ip::address ip, const unsigned short port,
                      const unsigned short threads /*= 1*/, Verbosity v /*= 0*/ )
    : m_ipAddress(ip)
    , m_nPort(port)
    , m_nThreads(threads)
    , m_verbosity(v)
    , m_localFunctor(onGetMessage)
{
    m_qListener = std::make_shared<Listener>(m_ipAddress, m_nPort, m_nThreads, v);

/*  None of this worked to create a functor on non static member function.
    m_qListener->setReporters( std::function<bool (const std::string&, std::string&)>(onGetMessage),
                               m_topMessageFunctor );

    auto lget = [this] (const std::string& sTarget, std::string& sOut) -> bool {
        onGetMessage(sTarget, sOut);
    };
*/

    ms_pInstance = this;
    m_qListener->setReporters( m_localFunctor, m_topMessageFunctor );
}

/*
WebServer::~WebServer()
{
    if (m_qListener) delete m_qListener;
}
*/

void  WebServer::setReporter(WebServerPostFunctor& pf)
{ 
    m_topMessageFunctor = pf;
    m_qListener->setReporters( m_localFunctor, m_topMessageFunctor );
}

bool WebServer::start()
{
    m_qListener->accept();
    m_qListener->run();
    return true;
}

int WebServer::onGetMessage(const std::string& inTarget, std::string& outPayload)
{
    std::string inPayload;
    return ms_pInstance->m_topMessageFunctor(inTarget, inPayload, outPayload);
}
