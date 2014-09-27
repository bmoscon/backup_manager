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

#ifndef __BACKUP_MANAGER__
#define __BACKUP_MANAGER__

#include <vector>
#include <string>

#include "defines.hpp"
#include "logger.hpp"


class BackupManager {
public:
    BackupManager(const std::vector<std::string>&, const std::string&, const logger_level,
		  const uint64_t&);

    void run(manager_state_e& state);

private:
    std::vector<std::string> _disks;
    Logger _log;
    uint64_t _interval;
};

#endif
