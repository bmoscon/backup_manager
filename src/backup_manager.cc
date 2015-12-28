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
 * 12/22/2015 - New design
 */

#include <algorithm>
#include <unistd.h>

#include "config_parse.hpp"
#include "backup_manager.hpp"
#include "common.hpp"


BackupManager::BackupManager(const std::string& cfg) 
{
    try {
	ConfigParse config(cfg);

	_log = new Logger(config.get_value("Settings", "log_path"));
	std::string level = config.get_value("Settings", "log_level");
	std::string ip = config.get_value("Settings", "db_ip");
	std::string pass = config.get_value("Settings", "db_pass");
	std::string user = config.get_value("Settings", "db_user");
	
	if (level.compare("DEBUG") == 0) {
	    _log->set_level(DEBUG);
	} else if (level.compare("INFO") == 0) {
	    _log->set_level(INFO);
	} else if (level.compare("WARNING") == 0) {
	    _log->set_level(WARNING);
	} else if (level.compare("ERROR") == 0) {
	    _log->set_level(ERROR);
	} else {
	    _log->set_level(INFO);
	}

	_db = new BackupManagerDB(ip, user, pass, _log);
	_db->init_tables();
	
	ConfigParse::const_iterator it = config.begin("Dirs");
	    
	for (; it != config.end("Dirs"); ++it) {
	    _disk_names.push_back(it->second);
	}
	    
    } catch (ConfigParseEx& e) {
	std::cerr << e.what() << std::endl;
	exit(EXIT_FAILURE);
    }

    _main_thread = std::thread(&BackupManager::worker, this);
}


BackupManager::~BackupManager()
{
    *_log << DEBUG << "Entering " << __PRETTY_FUNCTION__ << std::endl;

    set_state(SHUTDOWN);
    
    if (_main_thread.joinable()) {
	_main_thread.join();
    }

    *_log << DEBUG << "Leaving " << __PRETTY_FUNCTION__ << std::endl;

    delete _db;
    delete _log;
}


void BackupManager::init()
{
   *_log << DEBUG << "Entering " << __PRETTY_FUNCTION__ << std::endl;
   
    set_state(INIT);

    *_log << DEBUG << "Leaving " << __PRETTY_FUNCTION__ << std::endl;
}


void BackupManager::run()
{
    *_log << DEBUG << "Entering " << __PRETTY_FUNCTION__ << std::endl;
    
    set_state(RUN);
    
    *_log << DEBUG << "Leaving " << __PRETTY_FUNCTION__ << std::endl;
}


void BackupManager::wait()
{
    *_log << DEBUG << "Entering " << __PRETTY_FUNCTION__ << std::endl;
    
    set_state(WAIT);
    
    *_log << DEBUG << "Leaving " << __PRETTY_FUNCTION__ << std::endl;
}


void BackupManager::shutdown()
{
    *_log << DEBUG << "Entering " << __PRETTY_FUNCTION__ << std::endl;
    
    set_state(SHUTDOWN);
    
    *_log << DEBUG << "Leaving " << __PRETTY_FUNCTION__ << std::endl;
}


void BackupManager::worker()
{
    *_log << DEBUG << "Entering " << __PRETTY_FUNCTION__ << std::endl;
    
    Directory current_dir;
    
    while (_state != SHUTDOWN) {
	*_log << DEBUG << "Current state: " << state_to_str(_state) << std::endl;
	switch (_state) {
	case INIT:
	    setup_disks();
	    current_dir = next_dir();
	    wait();
	    break;
	case RUN:
	    check_dir(current_dir);
	    current_dir = next_dir();
	    if (current_dir.empty()) {
		wait();
	    }
	    break;
	case SHUTDOWN:
	case NONE:
	case WAIT:
	    break;
	default:
	    assert(false);
	}
	sleep(1);
    }

    *_log << DEBUG << "Leaving " << __PRETTY_FUNCTION__ << std::endl;
}


void BackupManager::setup_disks()
{
    *_log << DEBUG << "Entering " << __PRETTY_FUNCTION__ << std::endl;

    _disks.clear();
    
    for (uint32_t i = 0; i < _disk_names.size(); ++i) {
	_disks.push_back(Disk(_disk_names[i], _log));
    }

    *_log << DEBUG << "Leaving " << __PRETTY_FUNCTION__ << std::endl;
}


/* This method is responsible for returning the next valid directory
 * to check in the current mountpoint. If there are no more directories 
 * we should set the state to STOP to indicate we are done. 
 */
Directory BackupManager::next_dir()
{
    *_log << DEBUG << "Entering " << __PRETTY_FUNCTION__ << std::endl;

    Directory ret;
    
    if (_disks.size()) {
	ret = _disks[0].next_directory();
	if (ret.empty()) {
	    _disks.erase(_disks.begin());
	    *_log << DEBUG << "Leaving " << __PRETTY_FUNCTION__ << std::endl;
	    return next_dir();
	}
    }
    
    *_log << DEBUG << "Leaving " << __PRETTY_FUNCTION__ << std::endl;
    return (ret);
}


void BackupManager::check_dir(Directory& d)
{
    *_log << DEBUG << "Entering " << __PRETTY_FUNCTION__ << std::endl;
    if (d.empty()) {
	*_log << DEBUG << "Leaving " << __PRETTY_FUNCTION__ << std::endl;
	return;
    }

    Directory from_db = _db->get(d);

    if (!from_db.files.size()) {
	_db->insert(d);
    } else if (d != from_db) {
	if (from_db.files.size() > d.files.size()) {
	    *_log << WARNING << "Database entry for directory " << d.path << " has more files "
		" than disk" << std::endl;
	    for (file_cit it = from_db.files.cbegin(); it != from_db.files.cend(); ++it) {
		if (d.files.find(it->first) == d.files.end()) {
		    *_log << WARNING << "File " << it->second <<
			" is in DB but not on disk." << std::endl;
		}
	    }
	}
	for (file_it it = d.files.begin(); it != d.files.end(); ++it) {
	    file_it from_db_it = from_db.files.find(it->first);
	    if (from_db_it == from_db.files.end()) {
		_db->insert(it->second);
	    } else {
		if (from_db_it->second != it->second) {
		    *_log << WARNING << "File " << it->second << " does NOT match DB record!"
			  << std::endl;
		}
		it->second.checked = std::time(NULL);
		_db->update(it->second);
	    }   
	}
    }
    
    *_log << DEBUG << "Leaving " << __PRETTY_FUNCTION__ << std::endl;
}
