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
			     const logger_level lvl, const uint64_t& interval) : 
    _disks(disks), _log(log), _interval(interval)
{
    _log.set_level(lvl);
}

void BackupManager::run(manager_state_e& state)
{
    _log << DEBUG << "Entering run()" << std::endl;
    
    std::vector<Directory> files;
    std::vector<Disk> disks;

    do {
	if (files.empty() || files[0].files.empty()) {
	    disks.clear();
	    // create a disk object for each mount/disk
	    for (uint32_t i = 0; i < _disks.size(); ++i) {
		disks.push_back(Disk(_disks[i], &_log));
	    }
	}
       
	files.clear();
	for (uint32_t i = 0; i < disks.size(); ++i) {
	    files.push_back(disks[i].next_directory());
	}

	for (uint32_t i = 1; i < files.size(); ++i) {
	    if (files[i - 1].path.compare(files[i].path) != 0) {
		_log << WARNING << "Disk " << i - 1 << " and disk " << i << " mismatched " <<
		    "directories: " << files[i-1].path << ", " << files[i].path << std::endl;
	    }
	}
	
	// check if we have completed the disk
	if (files.empty() || files[0].files.empty()) {
	    // get completion time
	    time_t start_again = std::time(0) + _interval * 60;
	    
	    _log << INFO << "Scan of disks completed. Starting again in " << _interval << " minutes"
		 << std::endl;

	    while ((state != STOP) && (std::time(0) < start_again)) {
		sleep(30);
	    }
	    if (state != STOP) {
		_log << INFO << "New scan starting" << std::endl;
	    }
	}
	
	// check if we are in waiting state
	if (state == WAIT) {
	    _log << INFO << "In wait state. Suspending work" << std::endl;
	    while (state == WAIT) {
		// sleep until we are in a run or stop state
		sleep(30);
	    }
	    _log << INFO << "Wait state ended. Resuming work" << std::endl;
	}
    } while (state != STOP);
    
    _log << DEBUG << "Leaving run()" << std::endl;
}
