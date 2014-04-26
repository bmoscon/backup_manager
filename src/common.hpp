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

#ifndef __COMMON__
#define __COMMON__


#include <cstdint>

ssize_t readall(const int& fd, uint8_t *buffer, const ssize_t& len);



#endif
