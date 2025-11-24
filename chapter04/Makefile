#Shell and flags, mainly for powershell recipes
SHELL = pwsh.exe
.SHELLFLAGS = -NoLogo -NoProfile -Command

# Compiler and flags
CC     = gcc
CFLAGS = -std=c23 -Wall -Wextra -pedantic -g

#Where to put executables
BUILDDIR = build

# All .c files in this directory
SOURCES  = $(wildcard *.c)
TARGETS  = $(patsubst %.c,$(BUILDDIR)/%.exe,$(SOURCES))

.PHONY: all clean dirs

# Default target: build everything
all: dirs $(TARGETS)

# Ensure build directory exists
# change to mkdir -p $(BUILDDIR) on posix shells
dirs:
	New-Item -ItemType Directory -Path "$(BUILDDIR)" -Force | Out-Null

# Rule: source.c -> build/source.exe
$(BUILDDIR)/%.exe: %.c
	$(CC) $(CFLAGS) $< -o $@

# Convenience rule: treat "source.exe" as alias for full path "build/source.exe
# Use @: as a no-op command in posix shells
%.exe: $(BUILDDIR)/%.exe
	'' | Out-Null

# Clean: remove all .exe files in build/, ignore if none
# Change to rm -f $(targets) on posix shells
clean:
	if (Test-Path "$(BUILDDIR)") { Remove-Item "$(BUILDDIR)/*.exe" -ErrorAction SilentlyContinue }
