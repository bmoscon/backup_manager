CC = g++
CCFLAGS = -Wall -Werror -std=c++11 -ggdb3

HEADERS = $(wildcard *.hpp)
SRC     = $(wildcard *.cc)
OBJ     = $(subst .cc,.o,$(SRC))
LIBS    = -lpthread


all: backup_manager

clean:
	rm -f $(OBJ) backup_manager

backup_manager: $(OBJ)
	$(CC) -o backup_manager $(OBJ) $(LIBS)

%.o: %.cc $(HEADERS) Makefile
	$(CC) $(CCFLAGS) -c $< -o $@


