/* Backup Manager
 * 
 * Copyright (c) 2012-2014 Bryant Moscon - bmoscon@gmail.com
 * 
 * Please see the LICENSE file for the terms and conditions 
 * associated with this software.
 *
 *
 * 09/21/2014 - Initial open source release
 * 09/28/2014 - Handle missing files
 * 10/05/2014 - Initial DB integration
 *
 */

#include <algorithm>
#include <unistd.h>


#include "backup_manager.hpp"
#include "common.hpp"



BackupManager::BackupManager(const std::vector<std::string>& disks, const std::string& log, 
			     const logger_level lvl, const uint64_t& interval,
			     const std::string& user, const std::string& pass, 
			     const std::string& ip) 
    : _disks(disks), _log(log), _interval(interval), _db(ip, user, pass, &_log)
{
    _log.set_level(lvl);
}


void BackupManager::run(manager_state_e& state)
{
    _log << DEBUG << "Entering run()" << std::endl;
    
    std::vector<Directory> files;
    std::vector<Disk> disks;

    do {
	// check if this we are starting a new run
	if (files.empty() || 
	    std::all_of(files.begin(), files.end(), [](Directory d){return d.valid();})) {
	    disks.clear();
	    _reconcile.resize(_disks.size());
	    // create a disk object for each mount/disk
	    for (uint32_t i = 0; i < _disks.size(); ++i) {
		disks.push_back(Disk(_disks[i], &_log));
	    }
	}
       
	// populate a directory for each disk
	files.clear();
	for (uint32_t i = 0; i < disks.size(); ++i) {
	    files.push_back(disks[i].next_directory());
	    _log << INFO << "In loop " << std::endl;
	}

	db_check(files);
	dir_check(files);
	 
	
	// check if we have completed the disk
	if (files.empty() || 
	    std::all_of(files.begin(), files.end(), [](Directory d){return d.valid();})) {
	    // we're done with the scan so run the DB pruner
	    db_prune();
	    
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


/* Check files on disk against what is in the database
 *
 * Entry in DB for file? 
 * - Yes? - Do CRCs Match?
 * - - Yes - we're done
 * - - No - Do timestamps match?
 * - - - Yes? File is corrupted or otherwise damaged, see if we can restore it from a copy
 * - - - No? Update the database with the new timestamp and CRC
 * - No? - Add entry
 *
 * update last checked timestamp
 *
 */
void BackupManager::db_check(const std::vector<Directory>& dirs)
{
    for (uint32_t i = 0; i < dirs.size(); ++i) {
	Directory db_dir = _db.get(dirs[i]);
    }
}

/* DB Pruner
 *
 * Remove DB entries for files that were not checked. 
 * Presumably they were not checked because they have been deleted
 *
 */ 
void BackupManager::db_prune()
{
    
}


void BackupManager::dir_check(const std::vector<Directory>& dirs)
{
    _log << DEBUG << "Entering dir_check()" << std::endl;

    for (uint32_t i = 1; i < dirs.size(); ++i) { 
	// if the names do not match for these directories
	// we have a mismatch somewhere so we should save these off to reconcile at the end
	if (dirs[i-1].name.compare(dirs[i].name) != 0) {
	    _log << INFO << "Directory mismatch: " << dirs[i-1].path << ", " << dirs[i].path 
		 << std::endl;
	    _reconcile[i - 1].insert(std::make_pair(dirs[i-1].name, dirs[i-1]));
	    _reconcile[i].insert(std::make_pair(dirs[i].name, dirs[i]));
	    return;
	}

	// for each file in one directory, make sure its in the other, and that the CRCs and
	// file sizes match. If they do not, use the last modified date to determine which
	// is more up to date
	std::unordered_map<std::string, File>::const_iterator it = dirs[i-1].files.begin();
	std::unordered_map<std::string, File>::const_iterator found;
	while (it != dirs[i-1].files.end()) {
	    found = dirs[i].files.find(it->first);
	
	    if (found == dirs[i].files.end()) {
		// not found
		_log << INFO << "File " << it->first << " not present on " << dirs[i].path 
		     << std::endl;
		std::string from = dirs[i-1].path + "/" + it->second.name;
		std::string to = dirs[i].path + "/" + it->second.name;
		if (copystreambuff(from.c_str(), to.c_str())) {
		    _log << INFO << "Copied " << from << " to " << to << std::endl;
		} else { 
		    _log << ERROR << "Copying " << from << " to " << to << " failed" << std::endl;
		}
	    } else {
		if (found->second != it->second) {
		    // mismatch
		    _log << INFO << "File " << it->first << " on " << it->second.path << " and " << 
			found->second.path << " do not match" << std::endl;
		    // depending on how they do not match take an action
		    // 1. if crcs match, we can probably just ignore the error
		    // 2. if file sizes do not match, we need to take the newest file
		    //    and copy it over the older file
		    // 3. if crcs do not match, and file sizes match, or if crcs do not match
		    //    and sizes do not match, and dates are the same, we need to check the 
		    //    database. If we cannnot tell from the DB which is valid, we cannot
		    //    take any action.
		}
	    }
	    ++it;
	}


	// do the same as above, but in the opposite direction
	// i.e. previously we were checking that every file in dir[i] 
	// was in dir[i + 1], now lets check that every file in dir[i + 1] is 
	// in dir[i]
	it = dirs[i].files.begin();

	while (it != dirs[i].files.end()) {
	    found = dirs[i-1].files.find(it->first);
	
	    if (found == dirs[i-1].files.end()) {
		// not found
		_log << INFO << "File " << it->first << " not present on " << dirs[i-1].path 
		     << std::endl;
		std::string from = dirs[i].path + "/" + it->second.name;
		std::string to = dirs[i-1].path + "/" + it->second.name;
		if (copystreambuff(from.c_str(), to.c_str())) {
		    _log << INFO << "Copied " << from << " to " << to << std::endl;
		} else { 
		    _log << ERROR << "Copying " << from << " to " << to << " failed" << std::endl;
		}
	    } else {
		if (found->second != it->second) {
		    // mismatch
		    _log << INFO << "File " << it->first << " on " << it->second.path << " and " << 
			found->second.path << " do not match" << std::endl;
		    // depending on how they do not match take an action
		    // 1. if crcs match, we can probably just ignore the error
		    // 2. if file sizes do not match, we need to take the newest file
		    //    and copy it over the older file
		    // 3. if crcs do not match, and file sizes match, or if crcs do not match
		    //    and sizes do not match, and dates are the same, we need to check the 
		    //    database. If we cannnot tell from the DB which is valid, we cannot
		    //    take any action.
		}
	    }
	    ++it;
	}

    }

    _log << DEBUG << "Leaving dir_check()" << std::endl;
}
