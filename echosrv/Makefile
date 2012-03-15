CC = gcc
CFLAGS = -Wall -g -std=c99 -O2
LDFLAGS = 

SOURCES = echosrv.c srvmode.c network.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = echosrv

all: $(SOURCE) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@
.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
clean: 
	-rm -f $(OBJECTS)
	-rm -f echosrv

