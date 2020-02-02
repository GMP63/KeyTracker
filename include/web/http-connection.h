#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <functional>
#include <memory>

#include "verbosity.h"

#define MAX_BUFFER_SIZE       4096
#define STEADY_DEADLINE_TIMER   10


class HttpConnection : public std::enable_shared_from_this<HttpConnection>
{
public:

    using WebServerGetFunctor = std::function<int (const std::string&, std::string&)>;
    using WebServerPostFunctor = std::function<int (const std::string&, const std::string&, std::string&)>;

    static const size_t ms_maxBufferSize = MAX_BUFFER_SIZE;
//  static const int ms_steady_DeadlineTimer = STEADY_DEADLINE_TIMER;

    HttpConnection() = delete;

    HttpConnection( boost::asio::ip::tcp::socket & socket, Verbosity v = 0)
    : m_verbosity(v)
    , m_running(false)
    , m_shuttingDown(false)
    , m_writeReady(false)
    , m_socket(std::move(socket))
    , m_buffer(ms_maxBufferSize)
    {
    }

    void setReporters(WebServerGetFunctor& gf, WebServerPostFunctor& pf)
                      {m_getFunctor = gf; m_postFunctor = pf;}

    void run();
    bool isRunning()      { return m_running; }
    bool isShuttingDown() { return m_shuttingDown; }
    void stop();


private:

    void readRequest();
    void processRequest();
    void createGetResponse(boost::beast::http::response<boost::beast::http::dynamic_body> & response);
    void createPostResponse(boost::beast::http::response<boost::beast::http::dynamic_body> & response);
    void writeResponse();

    bool retrieveBody(std::string& sBody);
    void insertHtmlInResponse( boost::beast::http::response<boost::beast::http::dynamic_body> & response,
                               const char* payload,
                               const char* headTitle = nullptr,
                               const char* subject = nullptr );

    void insertTextInResponse( boost::beast::http::response<boost::beast::http::dynamic_body> & response,
                               const char* payload,
                               const char* prefix = nullptr,
                               const char* suffix = nullptr );

    // Verbosity or log level : 0:warnings & errors,  1:info,  2:trace,  3:debug
    const Verbosity              m_verbosity;

    // Set if this connection is active and performing a "burst" of request/response.
    // Reset at construction and when quitting after the connection is close.
    volatile bool                m_running;

    // Set when a shutdown request has just been received from the client.
    // Then a server is going to terminate.
    volatile bool                m_shuttingDown;

    // Set when a request has just been received and parsed OK.
    // Then writting a reply to the client is possible.
    volatile bool                m_writeReady;

    // The TCP stream socket for the currently connected client.
    boost::asio::ip::tcp::socket m_socket;

    // The abstract flat buffer (array) for performing reads.
    boost::beast::flat_buffer    m_buffer;

    // The request message.
    boost::beast::http::request<boost::beast::http::dynamic_body>   m_request;

    // The response message.
    boost::beast::http::response<boost::beast::http::dynamic_body>  m_response;

    // The GET functor to report upwards the GET requests.
    WebServerGetFunctor m_getFunctor;

    // The POST functor to report upwards the POST requests.
    WebServerPostFunctor m_postFunctor;
};

#endif // HTTPCONNECTION_H
