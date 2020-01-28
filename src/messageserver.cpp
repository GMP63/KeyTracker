/**
 * @file  MessageServer.cpp
 * @brief MessageServer implementation.
 *        This is the source/destination for all the messages to/from MapManager object.
 *        This implementation is around a generic web server agregation, but it can be easily
 *        adapted to work with any generic message source/destination.
 * @author Guillermo M. Paris
 * @date 2019-01-08
 */

#include <csignal>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <thread>

#include "backupmanager.h"
#include "messageserver.h"

MessageServer*  MessageServer::ms_pInstance(nullptr);

MessageServer::MessageServer( IMapManager* mgr, IMessageQueue* queue, BackupManager* bkp,
                              ICommunicationServer* comsrv, Verbosity v /* = 0 */ )
    : m_pMapManager(mgr)
    , m_pMessageQueue(queue)
    , m_pBackupManager(bkp)
    , m_pCommServer(comsrv)
    , m_topMessageServerFunctor(onMessage)
    , m_verbosity(v)
{
    ms_pInstance = this;

    // initialize the top event functor and set it to commServer
    m_pCommServer->setReporter(m_topMessageServerFunctor);
}

bool MessageServer::start()
{
    return m_pCommServer->start();
}

bool MessageServer::stop()
{
    return m_pCommServer->stop();
}

int MessageServer::onMessage(const std::string& inTarget, const std::string& inData, std::string& outData)
{
    if (inTarget == "keySent")
    {
        unsigned int port = 0;
        std::string  key, url;

        // split inData in 3 components
        size_t start = 0;
        size_t end = inData.find(payloadDelimiter);
        if (end == std::string::npos) // not found
        {
            key = inData.substr(start);
        }
        else // delimiter found
        {
            key = inData.substr(start, end - start);
            start = end + 1;
            end = inData.find(payloadDelimiter);
            if (end == std::string::npos) // not found
            {
                url = inData.substr(start);
            }
            else // another delimiter found
            {
                url = inData.substr(start, end - start);
                try
                {
                    port = std::stoi(inData.substr(1 + end));
                }
                catch(const std::invalid_argument&)
                {
                    port = 0;
                }
                catch(const std::out_of_range&)
                {
                    port = 0;
                }
            }
        }

        if (ms_pInstance->m_verbosity >= Verbosity::info)
            std::cout << "keySent :  Key=" << key << ", url=" << url
                      << ", port=" << port << '\n';

        Message* pMessage = new Message( Message::Command::addKey,
                                         std::move(key), std::move(url), 0, port );
//      std::thread alone( [&] () { // NOT IN SEPARATE THREAD ANYMORE
                ms_pInstance->m_pMessageQueue->push(pMessage);
//          } );
//      alone.detach(); // queue's pop() will get garbage pointers time to time if using separate thread ?!?!
        outData = "reported key sent : ";
        outData += key;
        return CMD_SUCCEEDED;
    }
    else if (inTarget == "isHotKey")
    {
        outData = ms_pInstance->isHotKey(inData) ? "YES" : "NO";

        if (ms_pInstance->m_verbosity >= Verbosity::info)
            std::cout << "isHotKey :  "
                      << inData << " :  " << outData << '\n';

        return CMD_SUCCEEDED;
    }
    else if (inTarget == "getTopHotKeys")
    {
        KeyFrequencyVector v;
        ms_pInstance->getTopHotkeys(v);
        std::stringstream ss;
        ss << "{["; // Build the proper json
        std::size_t max = (v.size() >= 10 ? 10 : v.size());
        for (std::size_t i = 0; i < max; i++)
        {
            ss << "{\"Key\": \"" << v[i].key << "\",\"Frequency\": " << v[i].frequency << '}';
            if (i < max - 1) ss << ',';
        }
        ss << "]}";
        outData = ss.str();

        if (ms_pInstance->m_verbosity >= Verbosity::info)
            std::cout << "getTopHotKeys :  " << ss.str() << '\n';

        return CMD_SUCCEEDED;
    }
    else if (inTarget == "totalKeys")
    {
        unsigned int num = ms_pInstance->getTotalKeys();
        outData = std::to_string(num);
        if (ms_pInstance->m_verbosity >= Verbosity::info)
            std::cout << "totalKeys :  "
                      << outData << '\n';

        return CMD_SUCCEEDED;
    }
    else if (inTarget == "setTopHotKeys" || inTarget == "setKeyReportBaseSize")
    {
        int num = 0;
        try
        {
            num = unsigned(std::stoi(inData));
        }
        catch(const std::invalid_argument&)
        {
            outData = "Invalid Argument: ";
            outData += inData;
            return CMD_FAILED;
        }
        catch(const std::out_of_range&)
        {
            outData = "Argument out of range: ";
            outData += inData;
            return CMD_FAILED;
        }

        if (num <= 0)
        {
            outData = "Argument should be greater than zero: ";
            outData += inData;
            return CMD_FAILED;
        }

        Message* pMessage = new Message( Message::Command::setRankingLength, 0, num, 0 );
/*      std::thread alone( [&] () { ms_pInstance->m_pMessageQueue->push(pMessage); } );
        alone.detach();  // queue's pop() will get garbage pointers time to time if using separate thread ?!?!
*/
        ms_pInstance->m_pMessageQueue->push(pMessage);
        outData = "Top key report size is now ";
        outData += inData;
        if (ms_pInstance->m_verbosity >= Verbosity::info)
            std::cout << "setKeyReportBaseSize :  Report size is now set to "
                      << inData << '\n';

        return CMD_SUCCEEDED;
    }
    else if (inTarget == "shutdown")
    {
        std::cout << (outData = "Shutdown in progress.") << '\n';
        std::raise(SIGQUIT);
        return CMD_SUCCEEDED;
    }
    else if (inTarget == "restart")
    {
        std::cout << (outData = "Restart in progress.") << '\n';
        std::raise(SIGHUP);
        return CMD_SUCCEEDED;
    }
    else if (inTarget == "restore")
    {
        bool retval = ms_pInstance->m_pBackupManager->restoreRequest();
        outData = retval ? "Restored OK." : "Restore FAILED.";
        return retval ? CMD_SUCCEEDED : CMD_FAILED;
    }
    else if (inTarget == "time")
    {
        using sc = std::chrono::system_clock;
        time_t tNow = sc::to_time_t(sc::now());
        std::_Put_time<char> dtManip = std::put_time(std::gmtime(&tNow), "%F %T");
        std::stringstream ss;
        ss << "Current time is <b>" << dtManip << "</b> UTC. It is <b>"
           << tNow << "</b> sec from epoch (01-01-1970).";

        if (ms_pInstance->m_verbosity >= Verbosity::info)
            std::cout << "time :  " << ss.str() << '\n';

        outData = ss.str();
        return CMD_SUCCEEDED;
    }
    else // unknown command string
    {
        if (ms_pInstance->m_verbosity >= Verbosity::info)
            std::cout << "Requested target not recognized : "
                      << inTarget << '\n';

        return CMD_UNKNOWN; // target not found
    }
}

