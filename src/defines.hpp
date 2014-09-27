/* Backup Manager Defines
 * 
 * Copyright (c) 2012-2014 Bryant Moscon - bmoscon@gmail.com
 * 
 * Please see the LICENSE file for the terms and conditions 
 * associated with this software.
 *
 *
 * 09/20/2014 - Initial open source release
 *
 */

#ifndef __DEFINES__
#define __DEFINES__

#include <vector>
#include <string>

#include "logger.hpp"


struct thread_data_st {
    std::string db_host;
    std::string db_user;
    std::string db_pass;
    logger_level log_level;
    uint64_t interval;
    std::vector<std::string> disks;
};


enum manager_state_e {
    RUN = 0,
    WAIT,
    STOP
};



#endif
