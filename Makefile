all: ac
#which compiler
CC = gcc

#where are include files kept
#INCLUDE = 

#options for development
CFLAGS = -g -Wall

#install directory
INSTDIR = 

OBJECTS = ac.o ac_slow.o ac_fast.o


ac: $(OBJECTS)
	$(CC) -o ac $(INCLUDE) $(CFLAGS) $(OBJECTS)
	
edit: $(OBJECTS)
	$(CC) -o $(OBJECTS) $(CFLAGS)
	
$(OBJECTS): ac.h



clean:
	rm -rf  $(OBJECTS) ac
