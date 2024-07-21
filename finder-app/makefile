# Makefile for compiling a C program
CROSS_COMPILE  = gcc
CFLAGS = -g -Wall
RM = rm -f

all: writer

writer: writer.c
	$(CROSS_COMPILE) $(CFLAGS) -o writer writer.c
clean:
	$(RM) writer
