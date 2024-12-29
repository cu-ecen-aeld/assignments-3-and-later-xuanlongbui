# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -pthread -g

# Source files
SRCS = main.c aesdsocket.c

# Object files (automatically derived from source files)
OBJS = $(SRCS:.c=.o)

# Output executable
TARGET = aesdsocket

# Default target
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(OBJS) $(TARGET)