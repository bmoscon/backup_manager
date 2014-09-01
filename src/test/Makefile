all: crc32 logger copy

crc32:
	g++ -Wall -o crc32_test crc32_test.cc ../crc32.cc ../common.cc -std=c++11 -I../ -lz

logger:
	g++ -Wall -o logger_test logger_test.cc ../logger.cc -std=c++11 -I../

copy:
	g++ -O3 -Wall -o copy_test copy_test.cc ../crc32.cc ../common.cc -std=c++11 -I../ 

clean:
	rm crc32_test logger_test copy_test
