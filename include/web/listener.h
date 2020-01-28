
#ifndef LISTENER_H
#define LISTENER_H

#include <boost/asio.hpp>
#include <functional>
#include <memory>

#include "verbosity.h"

#define MAX_THREADS       208

// Accepts incoming connections and launches the sessions.
class Listener : public std::enable_shared_from_this<Listener>
{
public:

    using WebServerGetFunctor = std::function<int (const std::string&, std::string&)>;
    using WebServerPostFunctor = std::function<int (const std::string&, const std::string&, std::string&)>;

    static const size_t max_threads = MAX_THREADS;

    Listener( boost::asio::ip::address address,
              unsigned short port, unsigned short threadQty, Verbosity v = 0 );
    ~Listener();

    void setReporters(WebServerGetFunctor& gf, WebServerPostFunctor& pf) {m_getFunctor = gf; m_postFunctor = pf;}

    boost::asio::io_context & getExecutionContext() { return m_ioc; }
    void accept();
    void run();
    void stop();


private:

    // Waits for all the thread finish.
    void waitThreadsToFinish();

    // Verbosity or log level : 0:warnings & errors,  1:info,  2:trace,  3:debug
    const Verbosity                 m_verbosity;

    // I/O context having the execution context.
    boost::asio::io_context         m_ioc;

    // Acceptor to give a new open socket to an external request.
    boost::asio::ip::tcp::acceptor  m_acceptor;

    // Number of threads to run the context. Must be less or equal than max_threads(MAX_THREADS)
    const unsigned short            m_threadQty;

    // Thread vector where the threads created by the accept/context can be controlled.
    std::vector<std::thread>        m_threads;

    // The GET functor to report upwards the GET requests.
    WebServerGetFunctor             m_getFunctor;

    // The POST functor to report upwards the POST requests.
    WebServerPostFunctor            m_postFunctor;
};

#endif // LISTENER_H
