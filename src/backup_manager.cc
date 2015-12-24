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

#include "disk.hpp"
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
	
	ConfigParse::const_iterator it = config.begin("Dirs");
	    
	for (; it != config.end("Dirs"); ++it) {
	    _disk_names.push_back(it->second);
	}
	    
    } catch (ConfigParseEx& e) {
	std::cerr << e.what() << std::endl;
	exit(EXIT_FAILURE);
    }
}


BackupManager::~BackupManager()
{
    *_log << DEBUG << "Entering " << __PRETTY_FUNCTION__ << std::endl;

    _state = SHUTDOWN;
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
   
    if (_main_thread.joinable()) {
	_main_thread.join();
    }

    _state = INIT;

    _main_thread = std::thread(&BackupManager::worker, this);

    *_log << DEBUG << "Leaving " << __PRETTY_FUNCTION__ << std::endl;
}

void BackupManager::run()
{
    _state = RUN;
}

void BackupManager::stop()
{
    _state = STOP;
}


void BackupManager::shutdown()
{
    _state = SHUTDOWN;
}


state_e BackupManager::get_state() const
{
    return (_state);
}

void BackupManager::worker()
{
    *_log << DEBUG << "Entering " << __PRETTY_FUNCTION__ << std::endl;
    
    while (_state != SHUTDOWN) {
	switch (_state) {
	case INIT:
	    setup_disks();
	    break;
	case RUN:
	    check_dir();
	    break;
	case STOP:
	case SHUTDOWN:
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

    for (uint32_t i = 0; i < _disk_names.size(); ++i) {
	_disks.push_back(Disk(_disk_names[i], _log));
    }
    
    *_log << DEBUG << "Leaving " << __PRETTY_FUNCTION__ << std::endl;
}


void BackupManager::check_dir()
{
    *_log << DEBUG << "Entering " << __PRETTY_FUNCTION__ << std::endl;

    
    *_log << DEBUG << "Leaving " << __PRETTY_FUNCTION__ << std::endl;
}
