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


#include <fstream>
#include <cstdio>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>

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


bool copyz(const char *in, const char *out, const uint32_t chunk)
{
    int p[2];

    if (pipe(p) != 0) {
	return (false);
    }

    int out_fd = open(out, O_RDWR | O_CREAT, 0777);
    int in_fd = open(in, O_RDONLY);
    
    if (in_fd < 0 || out_fd < 0) {
	return (false);
    }
    
    while(splice(p[0], 0, out_fd, 0, splice(in_fd, 0, p[1], 0, chunk, 0), 0)>0);

    close(out_fd);
    close(in_fd);

    return (true);
}



bool copyposix(const char *in, const char *out, const uint32_t chunk)
{
    char buffer[chunk];
    ssize_t ret;
    ssize_t rc;
    int out_fd = open(out, O_RDWR | O_CREAT, 0777);
    int in_fd = open(in, O_RDONLY);

    if (in_fd < 0 || out_fd < 0) {
	return (false);
    }
    
    while ((ret = read(in_fd, buffer, chunk)) > 0) {
	do {
	    rc = write(out_fd, buffer, ret);
	} while ((rc < 0) && (errno == EINTR));
	if (rc < 0) {
	    return (false);
	}
    }

    close(out_fd);
    close(in_fd);

    return (true);
}



bool copyansi(const char *in, const char *out, const uint32_t chunk)
{
   char buffer[chunk];
   size_t ret;

   FILE *src = fopen(in, "r");
   FILE *dst = fopen(out, "w");
   
   if (!src || !dst) {
       return (false);
   }

   while ((ret = fread(buffer, 1, chunk, src))) {
       fwrite(buffer, 1, ret, dst);
   }

   fclose(dst);
   fclose(src);

   return (true);
}


bool copylinux(const char *in, const char *out)
{
    int out_fd = open(out, O_RDWR | O_CREAT, 0777);
    int in_fd = open(in, O_RDONLY);
    
    struct stat stat_;
    fstat(in_fd, &stat_);

    sendfile(out_fd, in_fd, 0, stat_.st_size);

    close(out_fd);
    close(in_fd);

    return (true);
}


bool copystreambuff(const char *in, const char *out)
{
    std::ifstream src(in, std::ios::binary);
    std::ofstream dst(out, std::ios::binary);

    dst << src.rdbuf();

    dst.close();
    src.close();
 
    return (true);
}
