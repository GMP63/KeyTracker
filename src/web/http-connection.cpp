
#include <boost/beast/version.hpp>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include "web/http-connection.h"

namespace beast = boost::beast;    // from <boost/beast.hpp>
namespace http = beast::http;      // from <boost/beast/http.hpp>
namespace net = boost::asio;       // from <boost/asio.hpp>
namespace bsys_errc = boost::system::errc;
using bsys_error_code = boost::system::error_code;
using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>

// Initiate the synchronous operations associated with the connection.
void HttpConnection::run()
{
    m_running = true;
    while (m_running && !m_shuttingDown)
    {
        readRequest();
        if (!m_running) break;

        if (m_writeReady)
        {
            writeResponse();
        }
    }
}

void HttpConnection::stop()
{
    m_running = false;
    m_socket.close();
}

// Synchronously receive a complete request message.
void HttpConnection::readRequest()
{
    m_writeReady = false;
    m_buffer.consume(10000);
    auto self = shared_from_this();

    boost::beast::error_code ec;
    size_t bytes_read = http::read(m_socket, m_buffer, m_request, ec);
    if(ec)
    {
        if ( (  ec != http::error::end_of_stream &&
                ec.value() != bsys_errc::connection_reset &&
                ec.value() != bsys_errc::connection_aborted )
                || m_verbosity >= Verbosity::debug )
        {
            std::cerr << "\nRead error: "
                      << "code " << ec.value()
                      << ", reason: "<< ec.message()
                      << ", bytes read "
                      << bytes_read << "\n";
        }
        self->stop();
    }
    else if (bytes_read == 0) // should never happen
    {
        std::cerr << "GOOD NULL READ. Sleeping 1 mSec\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    else
    {
        if (m_verbosity >= Verbosity::trace)
            std::cout << "Procesing incomming request of " << bytes_read << " bytes.\n";

        self->processRequest();
    }
}

// Determine what needs to be done with the request message.
void HttpConnection::processRequest()
{
    http::response<http::dynamic_body> response; // workaround to get rid previous m_response not able to clear itself.

    response.set(http::field::server, "Boost Beast");
    response.version(m_request.version());
    response.keep_alive(true);

    switch(m_request.method())
    {
    case http::verb::get:
        response.result(http::status::ok);
        createGetResponse(response);
        break;

    case http::verb::post:
        response.result(http::status::no_content);
        createPostResponse(response);
        break;

    default:
        // Responses are returned indicating an error, if
        // we do not recognize the request method.
        if (m_verbosity >= Verbosity::info)
            std::cerr << "Invalid Request Method: "
                      << m_request.method_string() << std::endl;

        response.result(http::status::bad_request);
        response.set(http::field::content_type, "text/html");
        insertHtmlInResponse
        (
            response,
            std::string("Invalid request-method ''")
                        .insert(24, m_request.method_string().data()).c_str(),
            "Invalid Method",
            "Invalid Request Method:"
        );

        break;
    }

    m_buffer.clear();
    // Workaround to get rid previous m_response not able to clear itself.
    m_response = std::move(response);
    m_writeReady = true;
}

// Construct a response message based on the program state.
void HttpConnection::createGetResponse(http::response<http::dynamic_body> & response)
{
    if (m_request.target() == "/") // default GET reply
    {
        if (m_verbosity >= Verbosity::info)
            std::cout << "GET / (default or void request).\n"; // DEBUG 1 = info

        response.set(http::field::content_type, "text/html");

        std::ifstream indexFile("/home/guille/Documentos/work/workspace/boost_1_72_0/doc/html/index.html");
        if (indexFile.good())
        {
            std::string sIndexContent( (std::istreambuf_iterator<char>(indexFile) ),
                                       (std::istreambuf_iterator<char>()) );
            beast::ostream(response.body()) << sIndexContent;
        }
        else
        {
            response.result(http::status::not_found);
            insertHtmlInResponse( response, m_request.target().data(),
                                  "FILE NOT FOUND", "Inexistent index content." );
        }
    }
    else // report the GET request to upper layers
    {
        int retval = -1;
        std::string target(m_request.target());
        std::string reply;
        response.set(http::field::content_type, "text/html");
        if (target[0] == '/') target.erase(0, 1); // removes the leading '/'

        if ( (retval = m_getFunctor(target, reply)) >= 0)  // process the request
        {
            response.result(retval == 0 ? http::status::internal_server_error : http::status::ok);
            target.push_back(':');
            insertHtmlInResponse( response, reply.c_str(),
                                  "Reply from Hotspot Tracker", target.c_str() );
        }
        else // retval == -1 => target not found, or not recognized
        {
            response.result(http::status::not_found);
            insertHtmlInResponse( response, m_request.target().data(),
                                  "COMMAND NOT FOUND", "Inexistent or not recognized target." );
        }
    }
}

void HttpConnection::createPostResponse(http::response<http::dynamic_body> & response)
{
    bool found = false;
    int retval = -1;
    std::string target(m_request.target());
    std::string data;
    std::string reply;
    response.set(http::field::content_type, "text/html");

    if (retrieveBody(data))
    {
        found = ! data.empty();
    }

    if (!found)
        data = "ERROR: Argument(s) not found in the body.";

    if (target[0] == '/') target.erase(0, 1); // removes the leading '/'

    if ( found && (retval = m_postFunctor(target, data, reply)) >= 0 )  // process the request
    {
        target.push_back(':');
        response.result(retval == 0 ? http::status::internal_server_error : http::status::ok);
        insertHtmlInResponse( response, reply.c_str(),
                              "Reply from Hotspot Tracker", target.c_str() );
    }
    else
    {
        response.result(http::status::not_found);
        insertHtmlInResponse( response, m_request.target().data(),
                              "COMMAND OR PARAMETER NOT FOUND",
                              "Inexistent or not recognized target or supplied parameters." );
    }
}

// Synchronously transmit the response message.
void HttpConnection::writeResponse()
{
    auto self = shared_from_this();
    size_t size = m_response.body().size();

    m_response.set(http::field::content_length, size);

    if (m_verbosity >= Verbosity::debug)
        std::cout << "What it is about to send to the client: Sending response of "
                  << size << " body's bytes:\n\n"  << m_response << "\n\n";

    boost::beast::error_code ec;
    size_t bytes_written = http::write(m_socket, m_response, ec);
    if (ec)
    {
        std::cerr << "\nWrite error: "
                  << "code " << ec.value()
                  << ", reason: "<< ec.message()
                  << ", bytes written " << bytes_written
                  << "\n";
        self->stop();
        //self->m_socket.shutdown(tcp::socket::shutdown_send, ec);
    }

    if (m_verbosity >= Verbosity::trace)
        std::cout << bytes_written << " bytes were just sent.\n";

    m_response.clear(); // useless !!
}

bool HttpConnection::retrieveBody(std::string& sBody)
{
    size_t bodylength = std::stoul(m_request[http::field::content_length].data());
    if (bodylength == 0 || bodylength >= ms_maxBufferSize)
    {
        return false;
    }

    const char* pmsg = static_cast<const char*>( m_buffer.data().data() );
    size_t msglen = strlen(pmsg);
    if (msglen <= bodylength || msglen >= ms_maxBufferSize)
    {
        return false;
    }

    std::string_view   msgView(pmsg, msglen);
    const std::size_t  NPOS = std::string::npos;
    std::size_t        pos = msgView.rfind("Content-Length: ");
    if (pos == NPOS || (pos = msgView.find("\r\n", pos)) == NPOS)
    {
        return false;
    }

    if ((pos = msgView.find_first_not_of("\r\n", pos)) != NPOS)
    {
        sBody.assign(msgView, pos, bodylength);
        return true;
    }

    return false;
}

void HttpConnection::insertHtmlInResponse( http::response<http::dynamic_body> & response,
                                           const char* payload,
                                           const char* headTitle /* = nullptr */,
                                           const char* subject /* = nullptr */ )
{
    beast::ostream(response.body())
        << "<html>\n"
        << ( headTitle == nullptr ? "" : std::string("<head>\n<title></title>\n</head>\n").insert(14, headTitle))
        << "<body>\n"
        << ( subject == nullptr ? "" : std::string("<h1></h1>\n").insert(4, subject))
        << "<p>" << payload << "</p>\n</body>\n</html>\n";
}

void HttpConnection::insertTextInResponse( http::response<http::dynamic_body> & response,
                                           const char* payload,
                                           const char* prefix /* = nullptr */,
                                           const char* suffix /* = nullptr */ )
{
    beast::ostream(response.body()) << (prefix == nullptr ? "" : prefix)
                                    << payload
                                    << (suffix == nullptr ? "" : suffix);
}

