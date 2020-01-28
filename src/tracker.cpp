#include <boost/asio.hpp>
//#include <csignal>
#include <cstdlib>
#include <iostream>
#include <unistd.h>

#include <boost/beast.hpp>

#include "backupmanager.h"
#include "MapManager.h"
#include "messageserver.h"
#include "ThreadedMessageQueue.h"
#include "web/webserver.h"

#define DEFAULT_REPORTSIZE 10
#define MAX_REPORTSIZE     20
#define ONE_HOUR         3600

namespace net = boost::asio;    // from <boost/asio.hpp>
namespace beast = boost::beast; // from <boost/beast.hpp>

const size_t initialQueueNodes = 10000000; // 10M

int main(int argc, char* argv[])
{
    // 0:warnings & errors,  1:info,  2:trace,  3:debug
    Verbosity verbosity = 0;
    unsigned short threadQty = Listener::max_threads;

    // Check command line arguments.
    if(argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <IP address filter> <port> [threadsQTY] [-v[level]]\n";
        std::cerr << "  For IPv4, try:\n";
        std::cerr << "    receiver 0.0.0.0 8080\n";
        std::cerr << "  For IPv6, try:\n";
        std::cerr << "    receiver 0::0 8080\n\n";
        return EXIT_FAILURE;
    }

    if (argc >= 4)
    {
        unsigned short t = static_cast<unsigned short>(std::atoi(argv[3]));
        if (t > 0) threadQty = (threadQty > t ? t : threadQty);

        if (argc >= 5 && argv[4][0] == '-' && argv[4][1] == 'v' && '0' <= argv[4][2] && argv[4][2] <= '9')
        {
            verbosity = argv[4][2] - '0';
        }
    }

    if (verbosity >= Verbosity::info)
        std::cout << "\nRequired verbosity: " << int(verbosity) << "\n";

    auto const addressFilter = net::ip::make_address(argv[1]);
    unsigned short port = static_cast<unsigned short>(std::atoi(argv[2]));

    // Print pid, so that we can send signals from other shells.
    std::cout << "tracker pid is: " << getpid() << '\n';

    MapManager mapMgr(DEFAULT_REPORTSIZE, MAX_REPORTSIZE, verbosity);
    ThreadedMessageQueue queue(initialQueueNodes);
    queue.setConsumer(&mapMgr);
    BackupManager backupMgr(&mapMgr, ONE_HOUR, verbosity); // ONLY TEST: every 10 sec !!
    backupMgr.setFilenames("keys", "frequencies", "csv");
    WebServer webSrv(addressFilter, port, threadQty, verbosity);
    MessageServer msgSrv(&mapMgr, &queue, &backupMgr, &webSrv, verbosity);

    int signal = 0;
    // Capture SIGHUP for restarting this process, or SIGINT and SIGQUIT to perform a clean shutdown.
    net::signal_set signals(webSrv.getExecutionContext(), SIGHUP, SIGINT, SIGQUIT);
    signals.async_wait(
        [&](beast::error_code const&, int sig)
        {
            // Terminate the application.
            if (verbosity >= Verbosity::info)
                std::cout << "Stopping message server...\n";
            // Stop the `io_context`. This will cause `run()` to return
            // immediately, eventually destroying the`io_context`, all the
            // the open sockets in it, and stopping all the involved threads.
            signal = sig;
            msgSrv.stop();
        });

    if (verbosity >= Verbosity::info)
        std::cout << "Starting message Queue...\n";

    queue.start();

    if (verbosity >= Verbosity::info)
        std::cout << "Done.\nStarting Backup manager...\n";

    backupMgr.start();

    if (verbosity >= Verbosity::info)
        std::cout << "Done.\n";

    std::cout << "Server started.\nAccepting messages.\n";
    msgSrv.start(); // Starting the message server, so the IO context. Then, blocking main().

//  -----------------------------------------------------------------------  //

    // If we get here, it means we got a SIGHUP, SIGINT or SIGQUIT
    // Cleanup and close up stuff here
    if (verbosity >= Verbosity::info)
        std::cout << "Stopping backup manager...\n";

    backupMgr.stop();

    if (verbosity >= Verbosity::info)
        std::cout << "Done.\nStopping message queue...\n";

    queue.stop();

    if (verbosity >= Verbosity::info)
        std::cout << "Done.\n";

    // Terminate program: restart or shutdown
    const char* action = (signal == SIGHUP ? "Restarting" : "Exit");
    std::cout << "Server stopped.\n" << action << " tracker application.\n" << std::endl;
    return (signal == SIGHUP ? 128 + SIGHUP : EXIT_SUCCESS);
}

