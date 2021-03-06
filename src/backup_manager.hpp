/* Backup Manager
 * 
 * Copyright (c) 2012-2016 Bryant Moscon - bmoscon@gmail.com
 * 
 * Please see the LICENSE file for the terms and conditions 
 * associated with this software.
 *
 *
 * 09/21/2014 - Initial open source release
 * 12/22/2015 - New design
 */

#ifndef __BACKUP_MANAGER__
#define __BACKUP_MANAGER__

#include <vector>
#include <string>
#include <thread>

#include "schedulable.hpp"
#include "logger.hpp"
#include "db.hpp"
#include "disk.hpp"


class BackupManager : public Schedulable {
public:
    BackupManager(const std::string&);
    ~BackupManager();
    
    void init();
    void run();
    void wait();
    void shutdown();

private:
    void worker();
    void setup_disks();
    Directory next_dir();
    void check_dir(Directory&);
    
    std::thread _main_thread;
    std::vector<std::string> _disk_names;
    std::vector<Disk> _disks;
    Logger *_log;
    BackupManagerDB *_db;
};

#endif
