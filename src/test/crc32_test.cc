/* CRC32 Tester
 * 
 * Uses Zlib CRC32 to compare CRCs with custom CRC32 lib
 *
 * 
 * Copyright (c) 2014 Bryant Moscon - bmoscon@gmail.com
 * 
 * Please see the LICENSE file for the terms and conditions 
 * associated with this software.
 *
 *
 * 04/27/2014 - Initial open source release
 */

#include <iostream>
#include <cassert>
#include <fcntl.h>
#include <zlib.h>

#include "crc32.hpp"
#include "common.hpp"


uint32_t zlib_crc32(const char* filename)
{
    uint32_t crc = crc32(0L, Z_NULL, 0);
    int fd;
    int bytes_read;
    uint8_t buffer[1024];
    
    if ((fd = open(filename, O_RDONLY)) < 0) {
	return (0);
    }
    
    bytes_read = readall(fd, buffer, 1024);
    while (bytes_read > 0) {
	crc = crc32(crc, buffer, bytes_read);
	
	bytes_read = readall(fd, buffer, 1024);
	if (bytes_read < 0) {
	    close(fd);
	    return (0);
	}
    }
   
    close(fd);
    return (crc);
}


int main()
{
    CRC32 a(1024);
    
    assert(zlib_crc32("crc32_test") == a.crc32("crc32_test"));
    assert(zlib_crc32("crc32_test.cc") == a.crc32("crc32_test.cc"));

    std::cout << "*** PASS ***" << std::endl;
    return (0);
}
