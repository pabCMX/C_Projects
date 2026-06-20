# C Programming Projects

A structured learning repository for C programming, organized by chapter with example programs and exercises. Each chapter builds upon previous concepts, with comprehensive code reviews documenting progress and best practices.

## 📚 Repository Structure

```
C_Projects/
├── chapter02/              # C Fundamentals
│   ├── *.c                 # Example programs from the chapter
│   ├── Makefile
│   └── Exercises/
│       ├── ex1.c - ex8.c   # End-of-chapter exercises
│       ├── CODE_REVIEW.md  # Detailed code review with grades
│       ├── Makefile
│       └── build/          # Compiled executables
├── chapter03/              # Formatted Input/Output
├── chapter04/              # Expressions and Operators
├── chapter05/              # Selection Statements (if/switch)
├── Makefile.example        # Template Makefile with POSIX/PowerShell variants
├── .clangd                  # LSP configuration for editors
└── .clang-format           # Code formatting rules
```

---

## 🛠️ Getting Started

### Prerequisites

- **GCC** (GNU Compiler Collection) with C23 support
- **Make** (GNU Make or compatible)
- **PowerShell 7+** (Windows) or **Bash** (Linux/macOS)

Ensure `gcc` and `make` are in your system's `PATH`.

### Quick Start

```bash
# Navigate to a chapter's exercises
cd chapter05/Exercises

# Compile all exercises
make

# Compile a specific exercise
make ex1.exe

# Run the compiled program
./build/ex1.exe      # Linux/macOS
.\build\ex1.exe      # PowerShell
```

---

## 🔨 Build System Tutorial

### How Make Works

Make is a build automation tool that compiles only what's changed. It reads a `Makefile` containing rules that describe how to build targets from source files.

#### Basic Usage

```bash
make              # Build all .c files in the directory
make ex3.exe      # Build only ex3.c → build/ex3.exe
make clean        # Remove all compiled executables
```

#### How It Knows What to Rebuild

Make compares timestamps:
- If `ex3.c` is **newer** than `build/ex3.exe` → recompile
- If `ex3.c` is **older** than `build/ex3.exe` → skip (already up to date)

This saves time on large projects by only rebuilding what's necessary.

### Understanding the Makefile

```makefile
# Compiler and flags
CC     = gcc
CFLAGS = -std=c23 -Wall -Wextra -pedantic -g
```

| Flag | Purpose |
|------|---------|
| `-std=c23` | Use the C23 standard (enables modern features like digit separators `1'000`) |
| `-Wall` | Enable all common warnings |
| `-Wextra` | Enable extra warnings |
| `-pedantic` | Strict ISO C compliance warnings |
| `-g` | Include debug symbols (for debuggers like GDB) |

```makefile
# Where executables go
BUILDDIR = build

# Find all .c files automatically
SOURCES  = $(wildcard *.c)
TARGETS  = $(patsubst %.c,$(BUILDDIR)/%.exe,$(SOURCES))
```

The `wildcard` function finds all `.c` files, and `patsubst` transforms `ex1.c` → `build/ex1.exe`.

```makefile
# The compilation rule
$(BUILDDIR)/%.exe: %.c
    $(CC) $(CFLAGS) $< -o $@
```

| Symbol | Meaning |
|--------|---------|
| `$<` | The source file (e.g., `ex1.c`) |
| `$@` | The target file (e.g., `build/ex1.exe`) |
| `%` | Pattern wildcard (matches any filename) |

### Cross-Platform Support

The Makefiles in this repo automatically detect your operating system and use the appropriate shell commands. No manual configuration is needed—the same Makefile works on both Windows (PowerShell/cmd) and POSIX systems (Linux, macOS).

#### How It Works

The Makefile uses Make's built-in `OS` environment variable to detect the platform:

```makefile
# Detect platform via OS environment variable (Windows sets OS=Windows_NT)
ifeq ($(OS),Windows_NT)
    # Commands for Windows (calls PowerShell for complex operations)
    MKDIR_P = powershell -NoLogo -NoProfile -Command "New-Item -ItemType Directory -Path '$(BUILDDIR)' -Force | Out-Null"
    RM_EXE  = powershell -NoLogo -NoProfile -Command "if (Test-Path '$(BUILDDIR)') { Remove-Item '$(BUILDDIR)/*.exe' -ErrorAction SilentlyContinue }"
    NOOP    = @powershell -NoLogo -NoProfile -Command ""
else
    # Commands for POSIX shells (Linux, macOS, etc.)
    MKDIR_P = mkdir -p $(BUILDDIR)
    RM_EXE  = rm -f $(TARGETS)
    NOOP    = @:
endif
```

These platform-specific variables are then used in the targets:

```makefile
dirs:
    $(MKDIR_P)

%.exe: $(BUILDDIR)/%.exe
    $(NOOP)

clean:
    $(RM_EXE)
```

#### Platform-Specific Commands Reference

| Operation | POSIX (Bash) | Windows (cmd/PowerShell) |
|-----------|--------------|--------------------------|
| Create directory | `mkdir -p $(BUILDDIR)` | `powershell ... New-Item ...` |
| No-op command | `@:` | `@powershell ... ""` |
| Delete files | `rm -f $(TARGETS)` | `powershell ... Remove-Item ...` |

---

## 📝 Code Style & Editor Setup

This repo includes configuration files for consistent code style and editor intelligence:

- **`.clang-format`** – Defines code formatting rules (indentation, braces, spacing)
- **`.clangd`** – Shared compile flags (`-std=c23`, `-Wall`, etc.); no machine-specific include paths
- **`.vscode/settings.json`** – Cursor/VS Code format-on-save and clangd `--query-driver` allowlist
- **`.zed/settings.json`** – Zed format-on-save and the same `--query-driver` allowlist

### Cross-platform system headers

`.clangd` does not support “if Linux / if Windows” blocks — only path-based `If` conditions. Instead, clangd **asks your local GCC** where its system headers live via `--query-driver`. That list lives in `.vscode/settings.json` and `.zed/settings.json`, covering typical Linux (`/usr/bin/gcc`) and MSYS2 Windows (`C:/msys64/**/gcc.exe`) installs. No per-machine edits to `.clangd` are needed when switching machines.

If your GCC lives elsewhere (custom install path, different MSYS drive letter), add a glob to the `--query-driver` value in those editor settings files.

### VS Code / Cursor Setup

1. Install **clangd** (LLVM extension)
2. Install `clangd` and `clang-format` on your system (`apt install clangd clang-format` on Debian)
3. Open the repo — `.vscode/settings.json` is picked up automatically (format on save, query-driver, Microsoft IntelliSense disabled)

### Zed Setup

1. Install `clang-format` on your PATH (Zed can bundle clangd, or use a system install)
2. Open the repo — `.zed/settings.json` and `.clangd` are picked up automatically

### Command Line

If you prefer manual formatting:

```bash
# Format a single file in-place
clang-format -i ex1.c

# Format all .c files in current directory
clang-format -i *.c

# Check formatting without modifying (useful for CI)
clang-format --dry-run --Werror ex1.c
```

### What Clangd Provides

Once configured, clangd gives you:

| Feature | Description |
|---------|-------------|
| **Error highlighting** | Red squiggles under syntax errors as you type |
| **Warnings** | Yellow squiggles for potential issues (`-Wall`, `-Wextra`) |
| **Hover docs** | Hover over functions/types to see documentation |
| **Go to definition** | `Ctrl+Click` or `F12` to jump to definitions |
| **Auto-completion** | Intelligent suggestions as you type |
| **Rename symbol** | Rename variables/functions across files |

---

## 📊 Code Reviews

Each chapter's exercises include a `CODE_REVIEW.md` with:

- **Full source code** for each exercise
- **Strengths** – What was done well
- **Suggestions** – Potential improvements
- **Grade** – A+, A, A-, B+, etc.
- **Summary table** – Quick overview of all exercises

### Grading Criteria

| Grade | Meaning |
|-------|---------|
| A+ | Exceptional – creative solution or advanced technique |
| A | Excellent – correct, clean, well-documented |
| A- | Very good – minor style or edge case issues |
| B+ | Good – works correctly with some improvements needed |

---

## 🚀 Workflow Example

Here's a typical workflow for completing an exercise:

```bash
# 1. Navigate to the exercises folder
cd chapter05/Exercises

# 2. Make your solution.

# 3. Compile the script.
make ex9.exe

# 4. Run and test
./build/ex9.exe

# 5. Iterate until done/satisfied.

# 6. Compile all to make sure nothing broke
make
```

---

## 📁 Adding a New Chapter

1. Create the chapter directory:
   ```bash
   mkdir -p chapter06/Exercises/build
   ```

2. Copy over a Makefile:
   ```bash
   cp chapter05/Makefile chapter06/
   cp chapter05/Exercises/Makefile chapter06/Exercises/
   ```

3. Create your exercise files (`ex1.c`, `ex2.c`, etc.)

4. Build and test:
   ```bash
   cd chapter06/Exercises
   make
   ```

---

## 🔧 Troubleshooting

### "gcc: command not found"

GCC is not installed or not in PATH. 

**Windows:** Install via [MSYS2](https://www.msys2.org/) or [MinGW-w64](https://www.mingw-w64.org/)

**macOS:** Run `xcode-select --install`

**Linux:** `sudo apt install gcc` (Debian/Ubuntu) or equivalent

### "make: command not found"

**Windows:** Install via MSYS2 (`pacman -S make`) or use [GnuWin32](http://gnuwin32.sourceforge.net/packages/make.htm)

**macOS/Linux:** Usually included, or install via package manager

### PowerShell script execution errors

Run this once as Administrator:
```powershell
Set-ExecutionPolicy RemoteSigned -Scope CurrentUser
```

---

## 📜 License

This is a personal learning repository. Code is provided as-is for educational purposes.
