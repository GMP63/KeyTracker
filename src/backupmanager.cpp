#include <boost/filesystem.hpp>
#include <functional>
#include <iostream>
#include "IMapManager.h"
#include "backupmanager.h"

void BackupManager::setFilenames(const char* kfName, const char* ffName, const char* fExt)
{
    m_keyFilename = kfName;
    m_frequencyFilename = ffName;
    m_keyFilename += '.';
    m_frequencyFilename += '.';
    m_keyFilename += fExt;
    m_frequencyFilename += fExt;
}

void BackupManager::backupRequest()
{
    renameFiles();

    if (m_verbosity >= Verbosity::info)
        std::cout << "Done.\nPerforming backup of the maps ...\n";

    bool ok = m_mapMgr->backupRequest(m_keyFilename, m_frequencyFilename);
    if (!ok)
    {
        std::cerr << "Map backup FAILED!\n";
        return;
    }

    if (m_verbosity >= Verbosity::info)
        std::cout << "Done.\n";
}

bool BackupManager::restoreRequest()
{
    if (m_verbosity >= Verbosity::info)
        std::cout << "Done.\nPerforming restore of the maps ...\n";

    bool ret = m_mapMgr->restoreRequest(m_keyFilename, m_frequencyFilename);
    if (!ret)
    {
        std::cerr << "Map restore FAILED!\n";
    }
    else if (m_verbosity >= Verbosity::info)
        std::cout << "Done.\n";

    return ret;
}

bool BackupManager::start()
{
    if ( m_timer.getInterval() == 0 ||
         m_keyFilename.empty() || m_frequencyFilename.empty() )
        return false;

    m_timer.start(std::bind(&BackupManager::backupRequest, this));
    return true;
}

void BackupManager::stop()
{
    m_timer.stop();
}

/*
void BackupManager::operator () () const
{
    m_mapMgr->backupRequest("keys.csv", "frequencies.csv");
}
*/

void BackupManager::renameFiles()
{
    namespace fs = boost::filesystem;

    std::size_t posK = 0, posF = 0;
    const std::size_t NPOS = std::string::npos;
    std::string keyFName(m_keyFilename), freqFName(m_frequencyFilename);

    posK = keyFName.find_last_of('.');
    if (posK == NPOS) // should not happen because extension was added by construction.
        posK = keyFName.length();
    keyFName.insert(posK++, "_");

    posF = freqFName.find_last_of('.');
    if (posF == NPOS) // should not happen because extension was added by construction.
        posF = freqFName.length();
    freqFName.insert(posF++, "_");

    if (m_verbosity >= Verbosity::info)
        std::cout << "Renaming files ...\n";

    for (int i = 9; i > 0; --i)
    {
        std::string oldKeyFile(keyFName), newKeyFile(keyFName);
        std::string oldFreqFile(freqFName), newFreqFile(freqFName);

        if (i == 1)
            oldKeyFile = m_keyFilename;
        else
            oldKeyFile.insert(posK, 1, char('0' + i - 1));

        if (fs::exists(oldKeyFile))
        {
            newKeyFile.insert(posK, 1, char('0' + i));
            fs::rename(oldKeyFile, newKeyFile);
        }

        if (i == 1)
            oldFreqFile = m_frequencyFilename;
        else
            oldFreqFile.insert(posF, 1, char('0' + i - 1));

        if (fs::exists(oldFreqFile))
        {
            newFreqFile.insert(posF, 1, char('0' + i));
            fs::rename(oldFreqFile, newFreqFile);
        }
    }
}
