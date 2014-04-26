/* Common Functions
 * 
 * Copyright (c) 2012-2014 Bryant Moscon - bmoscon@gmail.com
 * 
 * Please see the LICENSE file for the terms and conditions 
 * associated with this software.
 *
 *
 * 04/26/2014 - Initial open source release
 *
 */


#include <errno.h>
#include <unistd.h>

#include "common.hpp"



ssize_t readall(const int& fd, uint8_t *buffer, const ssize_t& len)
{
    ssize_t bytes_read = 0;
    ssize_t total_read = 0;
    
    while((len != total_read) && (bytes_read = read(fd, buffer, len)) != 0) {
	if ((bytes_read == -1) && (errno != EINTR)) {
	    return (-1);
	}
	
	total_read += bytes_read;
	buffer += bytes_read;
    }
    
    return (total_read);
}


