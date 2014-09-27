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
 *
 */

#ifndef __FILE_OBJ__
#define __FILE_OBJ__

#include <string>
#include <vector>


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
    bool identical(const File&) const;
    bool valid() const;    
};


struct Directory {
    std::string        path;
    std::vector<File>  files;

    Directory(const std::string&, const std::vector<File>&);
};


#endif
