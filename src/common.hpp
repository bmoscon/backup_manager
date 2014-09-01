/* Common Functions
 * 
 * Copyright (c) 2012-2014 Bryant Moscon - bmoscon@gmail.com
 * 
 * Please see the LICENSE file for the terms and conditions 
 * associated with this software.
 *
 *
 * 04/26/2014 - Initial open source release
 * 09/01/2014 - Copy Routines
 *
 */

#ifndef __COMMON__
#define __COMMON__


#include <cstdint>

ssize_t readall(const int& fd, uint8_t *buffer, const ssize_t& len);

bool copyz(const char *in, const char *out, const uint32_t chunk);

bool copyposix(const char *in, const char *out, const uint32_t chunk);

bool copyansi(const char *in, const char *out, const uint32_t chunk);

bool copylinux(const char *in, const char *out);

bool copystreambuff(const char *in, const char *out);

#endif
