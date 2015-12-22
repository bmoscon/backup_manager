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

#include "disk.hpp"
#include "config_parse.hpp"
#include "backup_manager.hpp"
#include "common.hpp"



BackupManager::BackupManager(const std::string& cfg) 
{
    try {
	ConfigParse config(argv[2]);

	_log(config.getValue("Settings", "log_path"));
	std::string level = config.getValue("Settings", "log_level");
	std::string ip = config.getValue("Settings", "db_ip");
	std::string pass = config.getValue("Settings", "db_pass");
	std::string user = config.getValue("Settings", "db_user");
	
	if (level.compare("DEBUG") == 0) {
	    _log.set_level(DEBUG);
	} else if (level.compare("INFO") == 0) {
	    _log.set_level(INFO);
	} else if (level.compare("WARNING") == 0) {
	    _log.set_level(WARNING);
	} else if (level.compare("ERROR") == 0) {
	    _log.set_level(ERROR);
	} else {
	    _log.set_level(INFO);
	}

	_db(db_ip, user, pass, &_log);
	
	ConfigParse::const_iterator it = config.begin("Dirs");
	    
	for (; it != config.end("Dirs"); ++it) {
	    _disks.push_back(it->second);
	}
	    
    } catch (ConfigParseEx& e) {
	std::cerr << e.what() << std::endl;
	exit(EXIT_FAILURE);
    }
}


void BackupManager::init()
{
    if (_main_thread.joinable()) {
	_main_thread.join();
    }

    _state = INIT;

    _main_thread = std::thread(&BackupManager::worker, this);
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


void BackupManager::worker()
{
    while (_state != SHUTDOWN) {
	switch (_state) {
	case INIT:
	case RUN:
	case STOP:
	case SHUTDOWN:
	    break;
	default:
	    assert(false);
	}
	sleep(1);
    }
}
