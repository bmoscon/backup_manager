CC = g++
CCFLAGS = -Wall -Werror -std=c++14 -ggdb3 -I/usr/include/mysql

HEADERS = $(wildcard *.hpp)
SRC     = $(wildcard *.cc)
OBJ     = $(subst .cc,.o,$(SRC))
LIBS    = -lpthread -lmysqlclient -lmysqlcppconn


all: backup_manager

clean:
	rm -f $(OBJ) backup_manager

backup_manager: $(OBJ)
	$(CC) -o backup_manager $(OBJ) $(LIBS)

%.o: %.cc $(HEADERS) Makefile
	$(CC) $(CCFLAGS) -c $< -o $@


