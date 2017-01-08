/* Backup Manager Disk Object
 * 
 * Copyright (c) 2012-2015 Bryant Moscon - bmoscon@gmail.com
 * 
 * Please see the LICENSE file for the terms and conditions 
 * associated with this software.
 *
 *
 * 09/26/2014 - Initial open source release
 * 09/27/2014 - Directory support added
 *
 */

#ifndef __DISK__
#define __DISK__

#include <string>
#include <vector>

// would be nice to use ftw or nftw, but its a pain to 
// have to pass it a function pointer, which 
// in this case would have to be a static member
#include <dirent.h>

#include "file.hpp"
#include "logger.hpp"


class Disk {
public:
    Disk(const std::string&, Logger*);
    Directory next_directory();

private:
    std::string _mount;
    Logger* _log;
    std::vector<std::string> _to_process;
};

#endif
