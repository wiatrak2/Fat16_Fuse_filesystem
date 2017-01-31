CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra

filesystem: fat16.o fat16_fuse.o
	$(CC) $(CFLAGS) -o filesystem fat16.o fat16_fuse.o $(shell pkg-config fuse3 --cflags --libs)

fat16.o: fat16.c fat16.h

fat16_fuse.o: fat16_fuse.c fat16_fuse.h
	$(CC) $(shell pkg-config fuse3 --cflags --libs) fat16_fuse.c -c

clean:
	$(RM) filesystem *.o

.PHONY: clean