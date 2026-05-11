# Compiler and flags
CC      = gcc
CFLAGS  = -std=c23 -Wall -Wextra -pedantic -O3 -march=native -pthread
LDFLAGS = -pthread

# Where to put executables
BUILDDIR = build

# All .c files in this directory
SOURCES  = $(wildcard *.c)
TARGETS  = $(patsubst %.c,$(BUILDDIR)/%.exe,$(SOURCES))

# Detect platform via OS environment variable (Windows sets OS=Windows_NT)
ifeq ($(OS),Windows_NT)
    MKDIR_P = powershell -NoLogo -NoProfile -Command "New-Item -ItemType Directory -Path '$(BUILDDIR)' -Force | Out-Null"
    RM_EXE  = powershell -NoLogo -NoProfile -Command "if (Test-Path '$(BUILDDIR)') { Remove-Item '$(BUILDDIR)/*.exe' -ErrorAction SilentlyContinue }"
    NOOP    = @powershell -NoLogo -NoProfile -Command ""
else
    MKDIR_P = mkdir -p $(BUILDDIR)
    RM_EXE  = rm -f $(TARGETS)
    NOOP    = @:
endif

.PHONY: all clean dirs

# Default target: build everything
all: dirs $(TARGETS)

# Ensure build directory exists
dirs:
	$(MKDIR_P)

# Rule: source.c -> build/source.exe
$(BUILDDIR)/%.exe: %.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# Convenience rule: treat "source.exe" as alias for full path "build/source.exe"
%.exe: $(BUILDDIR)/%.exe
	$(NOOP)

# Clean: remove all .exe files in build/
clean:
	$(RM_EXE)
