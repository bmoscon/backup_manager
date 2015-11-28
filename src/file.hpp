/* Backup Manager File Object
 * 
 * Copyright (c) 2012-2015 Bryant Moscon - bmoscon@gmail.com
 * 
 * Please see the LICENSE file for the terms and conditions 
 * associated with this software.
 *
 *
 * 09/26/2014 - Initial open source release
 * 09/27/2014 - Directory object added
 * 09/28/2014 - Files in directory changed to hash map
 * 10/06/2014 - add new constructor for Directory
 * 11/26/2015 - various improvements
 * 11/27/2015 - directory comparison
 *
 */

#ifndef __FILE_OBJ__
#define __FILE_OBJ__

#include <string>
#include <unordered_map>
#include <ostream>


struct File {
    std::string path;
    std::string name;
    uint64_t    size;
    uint64_t    modified;
    uint32_t    crc;

    File();
    File(const std::string&, const std::string&, const uint64_t&, const uint64_t&, const uint32_t&);
    File(const std::string&, const std::string&);
    bool operator==(const File&) const;
    bool operator!=(const File&) const;
    bool identical(const File&) const;
    bool valid() const;    
};


struct Directory {
    std::string                            path;
    std::string                            name;
    std::unordered_map<std::string, File>  files;

    Directory();
    Directory(const std::string&, const std::string&, 
	      const std::unordered_map<std::string, File>&);

    bool empty() const;
    bool valid() const;
    bool operator==(const Directory&) const;
    bool identical(const Directory&) const;
};

typedef std::unordered_map<std::string, File>::const_iterator file_cit;

std::ostream& operator<<(std::ostream&, const Directory&);
std::ostream& operator<<(std::ostream&, const File&); 

#endif
