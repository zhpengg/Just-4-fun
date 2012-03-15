CC = gcc
CFLAGS = -Wall -g -std=gnu99 -O2
LDFLAGS = -lpthread

SOURCES = echosrv.c srvmode.c network.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = echosrv

all: $(SOURCE) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

benchmakr: benchmark.c
	$(CC) $(LDFLAGS) -o $@

clean: 
	-rm -f $(OBJECTS)
	-rm -f echosrv benchmark

