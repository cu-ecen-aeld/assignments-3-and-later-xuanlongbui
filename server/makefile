# Makefile for building a program with -lrt and -pthread

# Compiler to use
CC = gcc

# Compiler flags
CFLAGS = -Wall -g

# Linker flags (add -lrt for real-time library and -pthread for threading support)
LDFLAGS = -lrt -pthread

# Source files
SRC = main.c aesdsocket.c

# Output executable
OUTPUT = aesdsocket

# Default target
all: $(OUTPUT)

# Rule to build the executable
$(OUTPUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUTPUT) $(LDFLAGS)

# Clean rule to remove generated files
clean:
	rm -f $(OUTPUT)
