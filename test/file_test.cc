/* File/Directory/Disk object test code
 * 
 * 
 * Copyright (c) 2014 Bryant Moscon - bmoscon@gmail.com
 * 
 * Please see the LICENSE file for the terms and conditions 
 * associated with this software.
 *
 *
 * 09/27/2014 - Initial open source release
 */

#include <unordered_map>
#include <iostream>
#include <cassert>
#include <unistd.h>
#include <linux/limits.h>
#include <glob.h>

#include "disk.hpp"
#include "file.hpp"
#include "logger.hpp"
#include "crc32.hpp"



int main()
{
    std::string log_file;
    {
	char cwd[PATH_MAX];
	
	if (!getcwd(cwd, PATH_MAX)) {
	    perror("Get Current Working Directory failed");
	    return (1);
	}
	
	std::string dir(cwd);
	log_file = (dir +  "/tmp.log");
	Logger log(log_file);
	
	Disk disk(dir, &log);
	Directory files = disk.next_directory();

	assert(files.path.compare(cwd) == 0);

	std::unordered_map<std::string, File>::const_iterator it; 
	for (it = files.files.begin(); it != files.files.end(); ++it) {
	    CRC32 c(1024);
	    assert(it->second.crc == c.crc32(it->second.name));
	}

	glob_t g;
	size_t count;

	if (!glob((dir+"/*").c_str(), GLOB_NOSORT, NULL, &g)) {
	    count = g.gl_pathc;
	}
	
	globfree(&g);
	assert(count == files.files.size());
	
    }
    
    std::cout << "*** PASS ***" << std::endl;
    
    remove(log_file.c_str());
    return (0);
}
