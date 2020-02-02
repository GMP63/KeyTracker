#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

//#define SERVER_CLOSE_CONNECTION_AFTER_EVERY_REQUEST 1

namespace beast = boost::beast;  // from <boost/beast.hpp>
namespace http = beast::http;    // from <boost/beast/http.hpp>
namespace net = boost::asio;     // from <boost/asio.hpp>
using tcp = net::ip::tcp;        // from <boost/asio/ip/tcp.hpp>

const int   VERSION = 11; // HTTP 1.1
const char* host = "localhost";
const char* port = "8080";

std::mutex cprMutex;

bool connect(net::io_context & ioc, beast::tcp_stream & stream)
{
    tcp::resolver resolver(ioc);

    try
    {
        // Look up the domain name
        auto const results = resolver.resolve(host, port);
        // Make the connection on the IP address we get from a lookup
        stream.connect(results);
    }
    catch(const std::exception & e)
    {
        std::cerr << "Client connection error: " << e.what() << std::endl;
        return false;
    }

    return true;
}

void disconnect(beast::tcp_stream & stream)
{
    // Gracefully close the socket
    beast::error_code ec;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    // not_connected happens sometimes
    // so don't bother reporting it.
    //
    if(ec && ec != beast::errc::not_connected)
        throw beast::system_error{ec};

    // If we get here then the connection is closed gracefully
}


// -------- POST requests -------- //

void test_keySent( beast::tcp_stream & stream, beast::flat_buffer & buffer,
                   const char* key, bool log = true )
{
    clock_t start = 0, finish = 0;

    if (log)
    {
        std::cout << "Test for POST request to http://localhost:8080/keySent\n";
        start = clock();
    }

    http::request<http::string_body>  request(http::verb::post, "/keySent", VERSION);
    request.set(http::field::host, host);
    request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    request.set(http::field::content_type, "text/plain");
    request.body() = key;
    request.prepare_payload();

    // Send the HTTP request to the remote host
    http::write(stream, request);

    // Declare a container to hold the response
    http::response<http::dynamic_body>  response;

    // Receive the HTTP response
    http::read(stream, buffer, response);

    if (log) finish = clock();

    if (response.result() == http::status::ok)
    {
        if (log)      // Write the message to standard out
        {
            std::cout << "Response content: " << response << '\n'
                      << "It took " <<  (finish - start) << " uSec.\n" << std::endl;
        }
    }
    else
    {
        std::cerr << "Request error. Code: " << response.result_int() << std::endl;
    }

    buffer.clear();
}

void test_isHotKey( beast::tcp_stream & stream, beast::flat_buffer & buffer,
                    const char* key, bool log = true, bool multithread = false )
{
    clock_t start = 0, finish = 0;

    if (log)
    {
        std::cout << "Test for POST request to http://localhost:8080/isHotKey\n";
        start = clock();
    }

    http::request<http::string_body>  request(http::verb::post, "/isHotKey", VERSION);
    request.set(http::field::host, host);
    request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    request.set(http::field::content_type, "text/plain");
    request.body() = key;
    request.prepare_payload();

    if (multithread) cprMutex.lock();

    // Send the HTTP request to the remote host
    http::write(stream, request);

    // Declare a container to hold the response
    http::response<http::dynamic_body>  response;

    // Receive the HTTP response
    http::read(stream, buffer, response);

    if (multithread) cprMutex.unlock();

    if (log) finish = clock();

    if (response.result() == http::status::ok)
    {
        if (log)      // Write the message to standard out
        {
            std::cout << "Response content: " << response << '\n'
                      << "It took " <<  (finish - start) << " uSec.\n" << std::endl;
        }
    }
    else
    {
        std::cerr << "Request error. Code: " << response.result_int() << std::endl;
    }

    buffer.clear();
}

void test_setKeyReportBaseSize( beast::tcp_stream & stream, beast::flat_buffer & buffer,
                                unsigned int size, bool log = true)
{
    clock_t start = 0, finish = 0;

    if (log)
    {
        std::cout << "Test for POST request to http://localhost:8080/setKeyReportBaseSize\n";
        start = clock();
    }

    http::request<http::string_body>  request(http::verb::post, "/setKeyReportBaseSize", VERSION);
    request.set(http::field::host, host);
    request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    request.set(http::field::content_type, "text/plain");
    request.body() = std::to_string(size);
    request.prepare_payload();

    // Send the HTTP request to the remote host
    http::write(stream, request);

    // Declare a container to hold the response
    http::response<http::dynamic_body>  response;

    // Receive the HTTP response
    http::read(stream, buffer, response);

    if (log) finish = clock();

    if (response.result() == http::status::ok)
    {
        if (log)      // Write the message to standard out
        {
            std::cout << "Response content: " << response << '\n'
                      << "It took " <<  (finish - start) << " uSec.\n" << std::endl;
        }
    }
    else
    {
        std::cerr << "Request error. Code: " << response.result_int() << std::endl;
    }

    buffer.clear();
}


// -------- GET requests -------- //

void test_totalKeys( beast::tcp_stream & stream, beast::flat_buffer & buffer,
                     bool log = true )
{
    clock_t start = 0, finish = 0;

    if (log)
    {
        std::cout << "Test for GET request to http://localhost:8080/totalKeys" << std::endl;
        start = clock();
    }

    http::request<http::string_body>  request(http::verb::get, "/totalKeys", VERSION);
    request.set(http::field::host, host);
    request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Send the HTTP request to the remote host
    http::write(stream, request);

    // Declare a container to hold the response
    http::response<http::dynamic_body>  response;

    // Receive the HTTP response
    http::read(stream, buffer, response);

    if (log) finish = clock();

    if (response.result() == http::status::ok)
    {
        if (log)      // Write the message to standard out
        {
            std::cout << "Response content: " << response << '\n'
                      << "It took " <<  (finish - start) << " uSec.\n" << std::endl;
        }
    }
    else
    {
        std::cerr << "Request error. Code: " << response.result_int() << std::endl;
    }

    buffer.clear();
}

void test_getTopHotKeys( beast::tcp_stream & stream, beast::flat_buffer & buffer,
                         bool log = true )
{
    clock_t start = 0, finish = 0;

    if (log)
    {
        std::cout << "Test for GET request to http://localhost:8080/getTopHotKeys" << std::endl;
        start = clock();
    }

    http::request<http::string_body>  request(http::verb::get, "/getTopHotKeys", VERSION);
    request.set(http::field::host, host);
    request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Send the HTTP request to the remote host
    http::write(stream, request);

    // Declare a container to hold the response
    http::response<http::dynamic_body>  response;

    // Receive the HTTP response
    http::read(stream, buffer, response);

    if (log) finish = clock();

    if (response.result() == http::status::ok)
    {
        if (log)      // Write the message to standard out
        {
            std::cout << "Response content: " << response << '\n'
                      << "It took " <<  (finish - start) << " uSec.\n" << std::endl;
        }
    }
    else
    {
        std::cerr << "Request error. Code: " << response.result_int() << std::endl;
    }

    buffer.clear();
}

void tryConnect(net::io_context & ioc, beast::tcp_stream & stream, int step)
{
    // Try to establish the TCP connection
    if (! connect(ioc, stream))
    {
        std::cerr << "Step #" << ": Couldn't connect. Abort!\n"  << std::endl;
        exit(EXIT_FAILURE);
    }
}

#ifdef SERVER_CLOSE_CONNECTION_AFTER_EVERY_REQUEST
    #define TRY_CONNECT(close, ioc, strm, step) if (close) tryConnect(ioc, strm, step)
#else
    #define TRY_CONNECT(close, ioc, strm, step)
#endif

bool g_reconnect = false; // connection lives allowing undefined number of requests.

int main(int argc, char* argv[])
{
    if(argc > 1 && 0 == std::string(argv[1]).compare("--server-connection-lives-for-one-request"))
    {
        g_reconnect = true;
    }

    // The io_context is required for all I/O
    net::io_context ioc;

    // These objects perform our I/O
    beast::tcp_stream stream(ioc);

    // Try to establish the TCP connection
    tryConnect(ioc, stream, 0);

    // This buffer is used for reading and must be persisted
    beast::flat_buffer buffer;

    // POST request examples: isHotKey, keySent, setKeyReportBaseSize;
    try
    {
        TRY_CONNECT(g_reconnect, ioc, stream, 1);
        test_keySent(stream, buffer, "XXX-XXXXXX-1A");
        TRY_CONNECT(g_reconnect, ioc, stream, 1);
        test_keySent(stream, buffer, "XXX-XXXXXX-2B");
        TRY_CONNECT(g_reconnect, ioc, stream, 1);
        test_keySent(stream, buffer, "XXX-XXXXXX-3C");

        TRY_CONNECT(g_reconnect, ioc, stream, 2);
        test_setKeyReportBaseSize(stream, buffer, 15);

        TRY_CONNECT(g_reconnect, ioc, stream, 3);
        test_isHotKey(stream, buffer, "kkk");

        TRY_CONNECT(g_reconnect, ioc, stream, 4);
        test_keySent(stream, buffer, "YYY-YYYYYY-4");

        TRY_CONNECT(g_reconnect, ioc, stream, 5);
        test_isHotKey(stream, buffer, "YYY-YYYYYY-5");
    }
    catch(const std::exception & e)
    {
        std::cerr << "Client request error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    // GET request examples: totalKeys, getTopHotKeys;
    try
    {
        TRY_CONNECT(g_reconnect, ioc, stream, 6);
        test_getTopHotKeys(stream, buffer);

        TRY_CONNECT(g_reconnect, ioc, stream, 7);
        test_totalKeys(stream, buffer);

        TRY_CONNECT(g_reconnect, ioc, stream, 8);
        test_getTopHotKeys(stream, buffer);
    }
    catch(const std::exception & e)
    {
        std::cerr << "Client request error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "\nStress Test for massive multiple requests.\n\n";
    clock_t start = clock(), finish = 0;
/*  Sequential test
    bool blLog = false;
    char key[48];
    key[0] = key[47] = 0;
    for (int i = 0; i < 1000000; i++)
    {
        sprintf(key, "testing_keySent_%02i", i+1);
        TRY_CONNECT(ioc, stream, i + 10);
        test_keySent(stream, buffer, key, blLog);
    }
*/
    disconnect(stream); // throw !

//  Multithreaded test
    std::thread vt[200];
    for (int t = 0; t < 200; t++)
    {
        vt[t] = std::thread( [&ioc] (int t) {
            // These objects perform our I/O
            beast::tcp_stream stream(ioc);

            // Try to establish the TCP connection
            if (! connect(ioc, stream))
            {
                std::cerr << "ERROR! Thread #" << t << " could not connect. Bye!\n\n";
                return;
            }

            // This buffer is used for reading and must be persisted
            beast::flat_buffer buffer;

            bool blLog = false;
            char key[48];
            key[0] = key[47] = 0;
            for (int i = 0; i < 5000; i++)
            {
                sprintf(key, "testing_keySent_%03i_%04i", t+1, i+1);
                test_keySent(stream, buffer, key, blLog);
                TRY_CONNECT(g_reconnect, ioc, stream, 10000 * t + i);
            }
        }, t);
    }

    for (int t = 0; t < 200; t++) vt[t].join();

    disconnect(stream); // throw !
//
    finish = clock();
    std::cout << "LOOP TEST TOOK " << (finish - start)/1000 << " mSec .\n" << std::endl;

    return EXIT_SUCCESS;
}
