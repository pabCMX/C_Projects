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

### PowerShell vs POSIX Shells

The Makefiles in this repo use PowerShell on Windows. The `Makefile.example` is set up for POSIX (Bash/Linux) by default, with comments showing the PowerShell alternatives.

**To switch between shells, edit the following in the Makefile:**

#### 1. Shell Declaration (top of file)

```makefile
# POSIX (default - these lines commented out or absent):
# (uses /bin/sh by default)

# PowerShell (uncomment these):
SHELL = pwsh.exe
.SHELLFLAGS = -NoLogo -NoProfile -Command
```

#### 2. The `dirs` Target (create build directory)

```makefile
# POSIX:
dirs:
    mkdir -p $(BUILDDIR)

# PowerShell:
dirs:
    New-Item -ItemType Directory -Path "$(BUILDDIR)" -Force | Out-Null
```

#### 3. The `%.exe` Rule (no-op command)

```makefile
# POSIX:
%.exe: $(BUILDDIR)/%.exe
    @:

# PowerShell:
%.exe: $(BUILDDIR)/%.exe
    '' | Out-Null
```

#### 4. The `clean` Target (delete executables)

```makefile
# POSIX:
clean:
    rm -f $(TARGETS)

# PowerShell:
clean:
    if (Test-Path "$(BUILDDIR)") { Remove-Item "$(BUILDDIR)/*.exe" -ErrorAction SilentlyContinue }
```

#### Quick Reference Table

| Operation | POSIX (Bash) | PowerShell |
|-----------|--------------|------------|
| Shell declaration | *(default)* | `SHELL = pwsh.exe` + `.SHELLFLAGS = ...` |
| Create directory | `mkdir -p $(BUILDDIR)` | `New-Item -ItemType Directory -Path "$(BUILDDIR)" -Force \| Out-Null` |
| No-op command | `@:` | `'' \| Out-Null` |
| Delete files | `rm -f $(TARGETS)` | `if (Test-Path ...) { Remove-Item ... }` |

---

## 📝 Code Style & Editor Setup

This repo includes configuration files for consistent code style and editor intelligence:

- **`.clang-format`** – Defines code formatting rules (indentation, braces, spacing)
- **`.clangd`** – Configures the language server for linting and code navigation

### VS Code / Cursor Setup

#### Formatting

1. Install the **"C/C++"** extension (by Microsoft)
2. Open Settings (`Ctrl+,`) and enable **"Editor: Format On Save"**
3. The `.clang-format` file is automatically detected

#### Linting (Clangd)

1. Install the **"clangd"** extension (by LLVM)
2. Disable the IntelliSense from the C/C++ extension to avoid conflicts:
   - Open Settings → search for `C_Cpp.intelliSenseEngine`
   - Set to **"disabled"**
3. The `.clangd` file is automatically detected

**settings.json** (optional explicit config):
```json
{
  "editor.formatOnSave": true,
  "C_Cpp.intelliSenseEngine": "disabled",
  "clangd.path": "clangd",
  "clangd.arguments": ["--background-index", "--clang-tidy"]
}
```

### Zed Setup

#### Formatting

Add the following to your Zed `settings.json` (`Ctrl+,`):

```json
{
  "format_on_save": "on",
  "languages": {
    "C": {
      "formatter": {
        "external": {
          "command": "clang-format",
          "arguments": ["-assume-filename", "{buffer_path}"]
        }
      }
    }
  }
}
```

#### Linting (Clangd)

Zed automatically detects and uses clangd when it's installed. The `.clangd` file in the repo is picked up automatically.

**Note:** You may need to update the include paths in `.clangd` to match your system's GCC installation path.

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
