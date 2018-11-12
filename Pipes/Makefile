CC = gcc
CFLAGS = -Wall -pedantic -ansi
TARGETS = OSass parent child1 child2 child3

all: OSass

OSass : OSass.c
	$(CC) $(CFLAGS) -o ass1 OSass.c

clean:
	rm -rf $(TARGETS) *~ *.log
