# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -g

# Source files
SRCS = main.c scheduler.c memory.c filesystem.c disk.c

# Object files
OBJS = $(SRCS:.c=.o)

# Target executable name
TARGET = myos

# Default rule: build the target executable
all: $(TARGET)

# Rule to link object files into the target executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile a .c file into a .o file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to clean up compiled files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean
