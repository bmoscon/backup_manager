/* Copy Test/Benchmark Tool
 * 
 * Copyright (c) 2014 Bryant Moscon - bmoscon@gmail.com
 * 
 * Please see the LICENSE file for the terms and conditions 
 * associated with this software.
 *
 *
 * 05/03/2014 - Initial open source release
 * 06/07/2014 - Changed computation of wall time to include microseconds
 */

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdio>
#include <cassert>
#include <cmath>
#include <string>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "crc32.hpp"
#include "common.hpp"



#define OUT_FILE "/tmp/temp_out"


typedef void (*copyfp)(const char*, const char*, const uint32_t);

char buffer[1024 * 1024];
std::vector<size_t> files;
std::vector<size_t> chunks;
std::vector<std::pair<copyfp, std::string>> functions;



struct timeval time_diff(const struct timeval *start, const struct timeval *stop)
{
    struct timeval result;

    result.tv_sec = stop->tv_sec - start->tv_sec;

    if (stop->tv_usec < start->tv_usec) {
	result.tv_sec--;
	result.tv_usec = stop->tv_usec + 1000000 - start->tv_usec;
    } else {
	result.tv_usec = stop->tv_usec - start->tv_usec;
    }

    return (result);
} 


// fill buffer with random characters A-Z
static void buffer_init()
{
    for (uint32_t i = 0; i < sizeof(buffer); ++i) {
	buffer[i] = (char)((rand() % 26) + 65);
    }
}


static void copyz(const char *in, const char *out, const uint32_t chunk)
{
    int p[2];
    assert(pipe(p) != -1);
    int out_fd = open(out, O_RDWR | O_CREAT, 0777);
    int in_fd = open(in, O_RDONLY);
    assert(in_fd != -1);
    assert(out_fd != -1);
    
    while(splice(p[0], 0, out_fd, 0, splice(in_fd, 0, p[1], 0, chunk, 0), 0)>0);

    close(out_fd);
    close(in_fd);
}


static void copyposix(const char *in, const char *out, const uint32_t chunk)
{
    char buffer[chunk];
    ssize_t ret;
    int out_fd = open(out, O_RDWR | O_CREAT, 0777);
    int in_fd = open(in, O_RDONLY);

    assert(in_fd != -1);
    assert(out_fd != -1);
    
    while ((ret = read(in_fd, buffer, chunk)) > 0) {
	assert(write(out_fd, buffer, ret) != -1);
    }

    close(out_fd);
    close(in_fd);
}


static void copyansi(const char *in, const char *out, const uint32_t chunk)
{
   char buffer[chunk];
   size_t ret;

   FILE *src = fopen(in, "r");
   FILE *dst = fopen(out, "w");
   
   assert(src != NULL);
   assert(dst != NULL);

   while ((ret = fread(buffer, 1, chunk, src))) {
       fwrite(buffer, 1, ret, dst);
   }

   fclose(dst);
   fclose(src);
}


static void copylinux(const char *in, const char *out, const uint32_t UNUSED)
{
    int out_fd = open(out, O_RDWR | O_CREAT, 0777);
    int in_fd = open(in, O_RDONLY);
    
    struct stat stat_;
    fstat(in_fd, &stat_);

    sendfile(out_fd, in_fd, 0, stat_.st_size);

    close(out_fd);
    close(in_fd);
    
}


static void copystreambuff(const char *in, const char *out, const uint32_t UNUSED)
{
    std::ifstream src(in, std::ios::binary);
    std::ofstream dst(out, std::ios::binary);

    dst << src.rdbuf();

    dst.close();
    src.close();
    
}


static bool match(const char *in, const char *out)
{
    CRC32 test(4096);
    return (test.crc32(in) == test.crc32(out));
    
}



// Create a file of size megabytes
// filled with random data
static std::string create_file(size_t size) 
{
    std::string filename = "/tmp/copy_test_" + std::to_string(size);
    FILE *f;
    
    f = fopen(filename.c_str(), "w");
    assert(f != NULL);
    
    for (uint32_t i = 0; i < size; ++i) {
	fwrite(buffer, 1, sizeof(buffer), f);
    }
    
    fclose(f);
    return (filename);
}


int main()
{
    struct timeval start, end, result;

    buffer_init();

    functions.push_back(std::make_pair(copyz, "Zero Copy"));
    functions.push_back(std::make_pair(copyposix, "POSIX"));
    functions.push_back(std::make_pair(copyansi, "ANSI"));
    functions.push_back(std::make_pair(copylinux, "LINUX"));
    functions.push_back(std::make_pair(copystreambuff, "StreamBuff Copy"));

    files.push_back(512);
    files.push_back(1024);
    files.push_back(1024 * 2);

    chunks.push_back(1024 * 4);
    chunks.push_back(1024 * 32);
    chunks.push_back(1024 * 512);
    chunks.push_back(1024 * 1024);
    chunks.push_back(1024 * 1024 * 4);

    for (uint32_t fp = 0; fp < functions.size(); ++fp) {
	std::cout << "COPY TYPE: " << functions[fp].second << std::endl;
	std::cout << "file size in MB    seconds     \xC2\xB5seconds     chunk size    Match" << std::endl;
	std::cout << "===============================================================" << std::endl;
	for (uint32_t file = 0; file < files.size(); ++file) {
	    for (uint32_t i = 0; i < chunks.size(); ++i) {
		std::string fname = create_file(files[file]);
		gettimeofday(&start, NULL);
		functions[fp].first(fname.c_str(), OUT_FILE, chunks[i]);
		gettimeofday(&end, NULL);
		result = time_diff(&start, &end);
		std::cout << std::setw(15) << files[file] << std::setw(11)<< result.tv_sec;
		std::cout << std::setw(13) << result.tv_usec << std::setw(15) << chunks[i] << std::setw(7);
		std::cout << (match(fname.c_str(), OUT_FILE) ? "Yes" : "No")  << std::endl;
		assert(remove(OUT_FILE) == 0);
		assert(remove(fname.c_str()) == 0);
	    }
	}
    }
    
    return (0);
}
