all: crc32 logger copy

crc32:
	g++ -Wall -o crc32_test crc32_test.cc ../src/crc32.cc ../src/common.cc -std=c++11 -I../src/ -lz

logger:
	g++ -Wall -o logger_test logger_test.cc ../src/logger.cc -std=c++11 -I../src/

copy:
	g++ -O3 -Wall -o copy_test copy_test.cc ../src/crc32.cc ../src/common.cc -std=c++11 -I../src/ 

clean:
	rm crc32_test logger_test copy_test
