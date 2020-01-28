#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

// Performs an HTTP GET|POST and prints the response
int main(int argc, char* argv[])
{
    try
    {
        // Check command line arguments.
        if ( (argc != 5 && argc != 6) ||
             (argv[1][0] != 'G' &&  argv[1][0] != 'g' &&  argv[1][0] != 'P' && argv[1][0] != 'p') )
        {
            std::cerr <<
                "Usage: http-client-command <G|P<postedMsg>> <host> <port> <target> "
                        "[<HTTP version: 1.0 or 1.1(default)>]\n"
                      << "Example:\n"
                      << "    http-client-command G www.example.com 80 / 1.0\n"
                      << "    http-client-command G localhost 8080 /totalKeys"
                      << "    http-client-command Ppastoseco localhost 8080 /isHotKey\n";
            return EXIT_FAILURE;
        }

        bool const getAction = ( argv[1][0] == 'G' ||  argv[1][0] == 'g' ? true : false );
        std::string postMsg(getAction ? "" : &argv[1][1]);
        auto const host = argv[2];
        auto const port = argv[3];
        auto const target = argv[4];
        int version = argc == 6 && !std::strcmp("1.0", argv[5]) ? 10 : 11;

        // The io_context is required for all I/O
        net::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver(ioc);
        beast::tcp_stream stream(ioc);

        // Look up the domain name
        auto const results = resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup
        stream.connect(results);

        // Set up an HTTP GET/POST request message
        http::request<http::string_body>  req(getAction ? http::verb::get : http::verb::post, target, version);
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        if (!getAction)
        {
            req.set(http::field::content_type, "text/plain");
            req.body() = postMsg;
            req.prepare_payload();
        }

        // Send the HTTP request to the remote host
        http::write(stream, req);

        // This buffer is used for reading and must be persisted
        beast::flat_buffer buffer;

        // Declare a container to hold the response
        http::response<http::dynamic_body> res;

        // Receive the HTTP response
        http::read(stream, buffer, res);

        // Write the message to standard out
        std::cout << res << std::endl;

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
    catch(std::exception const& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
