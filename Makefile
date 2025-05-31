# Makefile for icsh project

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
OBJS = main.o command.o job.o signal.o

# Default target
all: icsh

# Link object files to create the final executable
icsh: $(OBJS)
	$(CC) $(CFLAGS) -o icsh $(OBJS)

# Compile each .c file into .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f *.o icsh
