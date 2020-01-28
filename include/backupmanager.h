#ifndef BACKUPMANAGER_H
#define BACKUPMANAGER_H

#include "timer/timer.h"
#include "verbosity.h"

#define BACKUP_TERM  3600 // seconds

class IMapManager;

class BackupManager
{
public:
    static const unsigned int s_backupTerm = BACKUP_TERM; // default timer lapse in seconds

    BackupManager() = delete;
    BackupManager(IMapManager* mgr, unsigned int term = s_backupTerm, Verbosity v = 0)
        : m_verbosity(v)
        , m_mapMgr(mgr)
        , m_backupTerm(term)
        , m_timer(1000 * term, true)
    {}

    void setFilenames(const char* kfName, const char* ffName, const char* fExt);

//  void operator () () const;
    void backupRequest();
    bool restoreRequest();
    bool start();
    void stop();

private:
    void renameFiles();

    // Verbosity or log level : 0:warnings & errors,  1:info,  2:trace,  3:debug
    const Verbosity m_verbosity;
    // seconds till backup take place
    unsigned int    m_backupTerm; // seconds
    IMapManager*    m_mapMgr; 
    Timer           m_timer;
    std::string     m_keyFilename;
    std::string     m_frequencyFilename;
};


#endif // BACKUPMANAGER_H
