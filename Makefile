# Makefile


## CC  = Compiler.
## CFLAGS = Compiler flags.
CC	= gcc
CFLAGS 	= -Wall  


## OBJ = Object files.
## SRC = Source files.
## EXE = Executable name.

SRC =		driver.c prime.c
OBJ =		driver.o prime.o
EXE = 		progName

all: server client
## Top level target is executable.
server:	server.o c4.o game_thread.o
		$(CC) $(CFLAGS) server.o c4.o game_thread.o -o server -pthread -lnsl
## Top level target is executable.
client:	client.c
		$(CC) client.c -o client -lnsl

## Dependencies

server.o: server.c game_thread.h
		gcc -c server.c -lpthread
game_thread.o: game_thread.c game_thread.h c4.h
		gcc -c game_thread.c -lpthread
c4.o: c4.h c4.c
		gcc -c c4.c
clean:
	rm -f *.o
clobber:
	rm -f *.o *.exe

		
	


