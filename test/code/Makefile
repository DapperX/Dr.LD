CC = gcc
 
CFLAGS = -fno-stack-protector -fno-asynchronous-unwind-tables
 
SOURCES = $(wildcard *.c)
OBJS  =  $(patsubst %.c,%.o,$(SOURCES))


.PHONY: all

all: $(OBJS)
	$(CC) $^ -o main

%.o: %.c
	$(CC) $< -c $(CFLAGS)

clean:	
	rm -f $(OBJS)