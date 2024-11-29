# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Source files
SRCS = aesdsocket.c

# Object files (generated from source files)
OBJS = $(SRCS:.c=.o)

# Executable name
EXEC = aesdsocket

# Default target (will be executed when 'make' is run with no arguments)
default: all

# 'all' target, which builds the executable
all: $(EXEC)

# Rule to build the executable from object files
$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC)

# Rule to compile .c files to .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and the executable
clean:
	rm -f $(OBJS) $(EXEC)

# Phony targets (not actual files, always executed when called)
.PHONY: all clean default
