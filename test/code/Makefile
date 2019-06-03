CC = gcc
 
CFLAGS = -fno-stack-protector -fno-asynchronous-unwind-tables
 
SOURCES = $(wildcard *.c)
OBJS  =  $(patsubst %.c,%.o,$(SOURCES))


.PHONY: all

all: $(OBJS)
 
%.o: %.c
	$(CC) $< -c $(CFLAGS)

clean:	
	rm -f $(OBJS)