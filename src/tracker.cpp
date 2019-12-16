#include <csignal>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <unistd.h>

#include "BackupManager.h"
#include "MapManager.h"
#include "MessageServer.h"
#include "ThreadedMessageQueue.h"

std::mutex exitMutex;
int running = 1;
int sigRecv = 0;
std::condition_variable exitCondition;

void signalHandler( int signum )
{
   std::cout << "Interrupt signal (" << signum << ") received.\n";

    // cleanup and close up stuff here  
    // terminate program
    running = 0;
    sigRecv = signum;
    std::cout << "Manager and Server notified to exit.\n";
    exitCondition.notify_all();
    sleep(1); // to let main to finish and avoid exception. This defect is introduced by agresive loop optimization
}

bool hasStoped() { return ! running; }

int main(int argc, char* argv[])
{
    // Print pid, so that we can send signals from other shells
    std::cout << "tracker pid is: " << getpid() << '\n';

    // register several signals to signal handler  
    signal(SIGHUP,  signalHandler);
    signal(SIGINT,  signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGUSR1, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGALRM, signalHandler);

    MapManager mgr;
    ThreadedMessageQueue queue;
    BackupManager backupMgr;
    //MessageServer srv;
    //srv.setQueue(&queue);
    //srv.setManager(&mgr);
    queue.start(&mgr);
    //srv.init();
    //backupMgr.start(&mgr);

    while (running)
    {
        std::unique_lock<std::mutex> lock(exitMutex);
        exitCondition.wait( lock, hasStoped );
    }

    // cleanup and close up stuff here
    // backupMgr.stop();
    // srv.finish();
    queue.stop();

    // terminate program
    //std::cout << "Exit tracker application\n";
    return sigRecv;
}


