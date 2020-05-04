
#include <iostream>

#include "web/http-connection.h"
#include "web/listener.h"

namespace beast = boost::beast;    // from <boost/beast.hpp>
namespace net = boost::asio;       // from <boost/asio.hpp>

using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>

Listener::Listener( net::ip::address address, unsigned short port,
                    unsigned short threadQty, Verbosity v /* = 0 */ )
    : m_verbosity(v)
    , m_ioc(threadQty)
    , m_acceptor(m_ioc, {address, port})
    , m_threadQty(threadQty)
{
/* // Not reusing socket address (port) by now.
        bsys_error_code ec;

        // Allow address reuse
        m_acceptor.set_option( net::socket_base::reuse_address(true), ec);
        if(ec)
        {
            std::cerr << "Could not set reuse address option on acceptor: "
                      << ec.message() << "\n";
            return;
        }
*/
}

Listener::~Listener()
{
    // Block until all the threads exit
    waitThreadsToFinish();
}

void Listener::accept()
{
    if (m_verbosity >= Verbosity::trace)
        std::cout << "listen called by thread id: "
                  << std::hex
                  << std::this_thread::get_id() << std::dec << '\n';

    auto self = shared_from_this();
    // The new connection gets its own strand.
    m_acceptor.async_accept( net::make_strand(m_ioc),
        [self](beast::error_code ec, tcp::socket socket)
        {
            if(ec)
            {
                // Should never happen.
                std::cerr << "Acceptor error: " << ec.message() << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            else
            {
                if (self->m_verbosity >= Verbosity::trace)
                    std::cout << "Starting connection in new thread id: "
                              << std::hex
                              << std::this_thread::get_id() << std::dec << '\n';

                std::shared_ptr<HttpConnection> http = std::make_shared<HttpConnection>(socket);
                http->setReporters(self->m_getFunctor, self->m_postFunctor);

                // Serve one arbitrary long request-response sequence until closed by remote (or shutdown)
                http->run();

                // If the command "shutdown" has just been received from the client.
                if (http->isShuttingDown())
                {
                    self->getExecutionContext().stop(); // stop the IO context
                    return;
                }

                if (self->m_verbosity >= Verbosity::trace)
                    std::cout << "Connection closed in thread id: "  << std::hex
                                << std::this_thread::get_id() << std::dec  << '\n';
            }

            if (self->m_verbosity >= Verbosity::trace)
                std::cout << "thread " << std::hex << std::this_thread::get_id()
                          << std::dec << " is about to listen again.\n";

            self->accept();
        });
}

void Listener::run()
{
    // Run the I/O service on the requested number of threads
    if (m_threadQty > 1)
    {
        m_threads.reserve(m_threadQty - 1);
        for(unsigned short i = m_threadQty - 1; i > 0; --i)
            m_threads.emplace_back( [&] {
                m_ioc.run();
            });
    }

    m_ioc.run();
}

void Listener::stop()
{
    // Stop the `io_context`. This will cause `run()`
    // to return immediately, eventually destroying the
    // `io_context` and all of the sockets in it.

    m_ioc.stop();
    waitThreadsToFinish();
}

void Listener::waitThreadsToFinish()
{
    if (m_threadQty > 1)
    {
        // Block until all the threads exit
        for(std::thread& t : m_threads)
        {
            if(t.get_id() != std::this_thread::get_id())
                t.join();
        }
    }
}
