all: crc32 logger copy file db

crc32:
	g++ -Wall -o crc32_test crc32_test.cc ../src/crc32.cc ../src/common.cc -std=c++11 -I../src/ -lz

logger:
	g++ -Wall -o logger_test logger_test.cc ../src/logger.cc -std=c++11 -I../src/

copy:
	g++ -O3 -Wall -o copy_test copy_test.cc ../src/crc32.cc ../src/common.cc -std=c++11 -I../src/ 

file:
	g++ -Wall -o file_test file_test.cc ../src/crc32.cc ../src/common.cc ../src/file.cc ../src/disk.cc ../src/logger.cc -std=c++11 -I../src/

db:
	g++ -Wall -o db_test db_test.cc ../src/crc32.cc ../src/common.cc ../src/file.cc ../src/disk.cc ../src/logger.cc ../src/db.cc -std=c++11 -I../src/ -I/usr/include/mysql -lmysqlclient -lmysqlcppconn

clean:
	rm -f crc32_test logger_test copy_test file_test db_test
