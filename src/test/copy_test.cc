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
#include <cstdio>
#include <cassert>
#include <cmath>
#include <string>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#include "common.hpp"



#define OUT_FILE "/tmp/temp_out"


char buffer[1024 * 1024];
std::vector<size_t> files;
std::vector<size_t> chunks;



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


static void copyz(const char *in, const char *out, uint32_t chunk)
{
    int p[2];
    pipe(p);
    int out_fd = open(out, O_WRONLY | O_CREAT);
    int in_fd = open(in, O_RDONLY);
    assert(in_fd != -1);
    assert(out_fd != -1);
    
    while(splice(p[0], 0, out_fd, 0, splice(in_fd, 0, p[1], 0, chunk, 0), 0)>0);

    close(out_fd);
    close(in_fd);
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

    files.push_back(512);
    files.push_back(1024);
    files.push_back(1024 * 5);

    chunks.push_back(1024 * 4);
    chunks.push_back(1024 * 32);
    chunks.push_back(1024 * 512);
    chunks.push_back(1024 * 1024);
    chunks.push_back(1024 * 1024 * 4);

    for (uint32_t file = 0; file < files.size(); ++file) {
	for (uint32_t i = 0; i < chunks.size(); ++i) {
	    std::string fname = create_file(files[file]);
	    gettimeofday(&start, NULL);
	    copyz(fname.c_str(), OUT_FILE, chunks[i]);
	    gettimeofday(&end, NULL);
	    result = time_diff(&start, &end);
	    std::cout << "File of size " << files[file] << " took " << result.tv_sec;
	    std::cout << " seconds and " << result.tv_usec << " microseconds with chunk size ";
	    std::cout << chunks[i] << std::endl;
	    assert(remove(OUT_FILE) == 0);
	    assert(remove(fname.c_str()) == 0);
	}
    }
    
    
    return (0);
}
