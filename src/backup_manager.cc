/* Backup Manager
 * 
 * Copyright (c) 2012-2014 Bryant Moscon - bmoscon@gmail.com
 * 
 * Please see the LICENSE file for the terms and conditions 
 * associated with this software.
 *
 *
 * 09/21/2014 - Initial open source release
 *
 */

#include "backup_manager.hpp"
#include "disk.hpp"

#include <unistd.h>

BackupManager::BackupManager(const std::vector<std::string>& disks, const std::string& log, 
			     const logger_level lvl) : 
    _disks(disks), _log(log) 
{
    _log.set_level(lvl);
}

void BackupManager::run(manager_state_e& state)
{
    _log << DEBUG << "Entering run()" << std::endl;
    while (state == RUN) {
	sleep(5);
    }
     _log << DEBUG << "Leaving run()" << std::endl;
}
