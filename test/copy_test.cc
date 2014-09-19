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
#include <iomanip>
#include <cassert>
#include <cmath>
#include <string>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#include "crc32.hpp"
#include "common.hpp"



#define OUT_FILE "/tmp/temp_out"


typedef bool (*copyfp_chunk)(const char*, const char*, const uint32_t);
typedef bool (*copyfp)(const char*, const char*);

typedef struct copy_type_st {
    bool             chunk;
    union {
	copyfp_chunk fp_chunk;
	copyfp       fp;
    };
    std::string     name; 
} copy_type_st;

char buffer[1024 * 1024];
std::vector<size_t> files;
std::vector<size_t> chunks;
std::vector<copy_type_st> functions;



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
    copy_type_st copy;

    buffer_init();

    copy.chunk = true;
    copy.name = "Zero Copy";
    copy.fp_chunk = copyz;
    functions.push_back(copy);
    
    copy.name = "POSIX";
    copy.fp_chunk = copyposix;
    functions.push_back(copy);
    
    copy.name = "ANSI";
    copy.fp_chunk = copyansi;
    functions.push_back(copy);

    copy.chunk = false;
    copy.name = "Linux";
    copy.fp = copylinux;
    functions.push_back(copy);

    copy.name = "StreamBuffer";
    copy.fp = copystreambuff;
    functions.push_back(copy);

    files.push_back(512);
    files.push_back(1024);
    files.push_back(1024 * 2);

    chunks.push_back(1024 * 4);
    chunks.push_back(1024 * 32);
    chunks.push_back(1024 * 512);
    chunks.push_back(1024 * 1024);
    chunks.push_back(1024 * 1024 * 4);

    for (uint32_t fp = 0; fp < functions.size(); ++fp) {
	std::cout << "COPY TYPE: " << functions[fp].name << std::endl;
	std::cout << "file size in MB    seconds     \xC2\xB5seconds     chunk size    Match" << std::endl;
	std::cout << "===============================================================" << std::endl;
	for (uint32_t file = 0; file < files.size(); ++file) {
	    for (uint32_t i = 0; i < chunks.size(); ++i) {
		std::string fname = create_file(files[file]);
		gettimeofday(&start, NULL);
		if (functions[fp].chunk) {
		    functions[fp].fp_chunk(fname.c_str(), OUT_FILE, chunks[i]);
		} else {
		    functions[fp].fp(fname.c_str(), OUT_FILE);
		}
		gettimeofday(&end, NULL);
		result = time_diff(&start, &end);
		std::cout << std::setw(15) << files[file] << std::setw(11)<< result.tv_sec;
		std::cout << std::setw(13) << result.tv_usec << std::setw(15) << (functions[fp].chunk ? std::to_string(chunks[i]) : "NA") << std::setw(9);
		std::cout << (match(fname.c_str(), OUT_FILE) ? "Yes" : "No")  << std::endl;
		assert(remove(OUT_FILE) == 0);
		assert(remove(fname.c_str()) == 0);

		if (!functions[fp].chunk) {
		    break;
		}
	    }
	}
    }
    
    return (0);
}
