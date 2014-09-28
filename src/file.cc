/* Backup Manager File Object
 * 
 * Copyright (c) 2012-2014 Bryant Moscon - bmoscon@gmail.com
 * 
 * Please see the LICENSE file for the terms and conditions 
 * associated with this software.
 *
 *
 * 09/26/2014 - Initial open source release
 * 09/27/2014 - Directory object added
 * 09/28/2014 - Files in directory changed to hash map
 *
 */

#include <sys/stat.h>

#include "file.hpp"
#include "crc32.hpp"



File::File() : path(""), name(""), size(0), modified(0), crc(0) {}


File::File(const std::string& p, const std::string& n, const uint64_t& s, const uint64_t& m, 
	   const uint32_t& c) : path(p), 
				name(n), 
				size(s), 
				modified(m), 
				crc(c) {}


File::File(const std::string& p, const std::string& n)
{
    path = p;
    name = n;
    
    std::string full_path = p + n;
    
    CRC32 c(4096);
    crc = c.crc32(full_path);
    
    struct stat s;
    stat(full_path.c_str(), &s);
    
    size = s.st_size;
    modified = s.st_mtime;
}


bool File::operator==(const File& f) const
{
    return ((size == f.size) && (modified == f.modified) && (crc == f.crc) && 
	    (name.compare(f.name) == 0));
}


bool File::operator!=(const File& f) const
{
    return (!((*this) == f));
}


bool File::identical(const File& f) const
{
    return (((*this) == f) && (path.compare(f.path) == 0));
}


bool File::valid() const
{
    return (path.empty() || name.empty());
}


Directory::Directory(const std::string& p, const std::string& n, 
		     const std::unordered_map<std::string, File>& f) : 
    path(p), name(n), files(f) {}


bool Directory::empty() const
{
    return (this->files.empty());
}
