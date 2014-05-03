#include <iostream>
#include <cstdio>
#include <cassert>
#include <cmath>
#include <string>
#include <vector>
#include <ctime>
#include <fcntl.h>
#include <unistd.h>

#include "common.hpp"



#define OUT_FILE "/tmp/temp_out"


char buffer[1024 * 1024];
std::vector<size_t> files;
std::vector<size_t> chunks;



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
    time_t start, end;
    double diff;

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
	    time(&start);
	    copyz(fname.c_str(), OUT_FILE, chunks[i]);
	    time(&end);
	    diff = difftime(end, start);
	    std::cout << "File of size " << files[file] << " took " << diff << " seconds with chunk size " << chunks[i] << std::endl;
	    assert(remove(OUT_FILE) == 0);
	    assert(remove(fname.c_str()) == 0);
	}
    }
    
    
    return (0);
}
