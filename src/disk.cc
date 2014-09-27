/* Backup Manager Disk Object
 * 
 * Copyright (c) 2012-2014 Bryant Moscon - bmoscon@gmail.com
 * 
 * Please see the LICENSE file for the terms and conditions 
 * associated with this software.
 *
 *
 * 09/26/2014 - Initial open source release
 * 09/27/2014 - Directory support added
 *
 */

#include <cstring>

// would be nice to use ftw or nftw, but its a pain to 
// have to pass it a function pointer, which 
// in this case would have to be a static member
#include <dirent.h>

#include "disk.hpp"



Disk::Disk(const std::string& mount, Logger *log) : _mount(mount), _log(log) 
{
    _to_process.push_back(mount);
}

Directory Disk::next_directory()
{
    DIR *dir;
    struct dirent *entry;
    std::vector<File> files;
    std::string path;
    
    // need to keep checking until files is non-empty
    // in case we hit directories that have no files in them.
    // we only want to return an empty vector when we are
    // really done processing ALL files files underneath the
    // given mountpoint
    while (!_to_process.empty() && files.empty()) {
	
	path = _to_process.back();
	dir = opendir(path.c_str());
	_to_process.pop_back();
	(*_log) << DEBUG << "Processing " << path << std::endl;
	
	if (!dir) {
	    (*_log) << ERROR << "Cannot open " << path << std::endl;
	} else {
	    
	    while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type == DT_REG) {
		    files.push_back(File(path + "/", entry->d_name));
		} else if ((entry->d_type == DT_DIR) && 
			   (strcmp(entry->d_name, ".") != 0) &&
			   (strcmp(entry->d_name, "..") != 0)) {
		    _to_process.push_back(path + "/" + entry->d_name);
		}
	    }
	    
	    closedir(dir);
	}
    }
    
    Directory d(path, files);
    
    return (d);
}
