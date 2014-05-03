all: crc32 logger copy

crc32:
	g++ -o crc32_test crc32_test.cc ../crc32.cc ../common.cc -std=c++11 -I../ -lz

logger:
	g++ -o logger_test logger_test.cc ../logger.cc -std=c++11 -I../

copy:
	g++ -o copy_test copy_test.cc ../common.cc -std=c++11 -I../

clean:
	rm crc32_test logger_test
