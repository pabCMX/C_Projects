# Compiler and flags
CC     = gcc
CFLAGS = -std=c23 -Wall -Wextra -pedantic -g

# All .c files in this directory
SOURCES  = $(wildcard *.c)
TARGETS  = $(SOURCES:.c=.exe)

# Default target: build everything
all: $(TARGETS)

# Rule: .c -> .exe
%.exe: %.c
	$(CC) $(CFLAGS) $< -o $@

# Clean: remove all .exe files (and any .o if you add them later)
clean:
	rm -f $(TARGETS)
