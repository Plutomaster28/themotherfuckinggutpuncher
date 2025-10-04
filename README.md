# GutPuncher

**Aggressive Malware Removal Tool**

GutPuncher is a powerful, no-nonsense malware removal tool specifically designed to hunt down and eliminate the Comet AI Browser and associated computer worms. Built in C and compiled as a static executable, it works on any Windows machine without dependencies.

## Features

- **Process Termination**: Aggressively terminates malware processes with respawn detection
- **File System Cleanup**: Recursively deletes malware directories and files
- **Registry Cleaning**: Removes malware registry keys and entries
- **Startup Cleanup**: Eliminates malware from Windows startup locations
- **Scheduled Task Removal**: Cleans malware scheduled tasks
- **Static Build**: Works on any Windows machine without runtime dependencies
- **Admin Privilege Escalation**: Uses debug privileges for maximum effectiveness

## Targets

### Processes
- `comet.exe`
- `cometbrowser.exe`
- `comet-ai.exe`
- `cometai.exe`
- `comet_browser.exe`
- `comet_update.exe`
- `cometupdater.exe`
- `comet_service.exe`
- Any process containing "comet" (fuzzy matching)

### Locations
- Program Files
- Program Files (x86)
- AppData (Roaming)
- AppData (Local)
- Common AppData
- Registry (HKCU and HKLM)
- Startup folders
- Scheduled Tasks

## Build Status

**SUCCESSFULLY COMPILED** with GCC 15.2.0 UCRT64 + Ninja!

Executable: `build/bin/gutpuncher.exe` (static, optimized, portable)

## Building

### Prerequisites
- **CMake** (3.15 or higher)
- **Ninja** (recommended, but optional)
- **UCRT64 GCC** (MinGW-w64 with UCRT runtime)

### Quick Build (Recommended)

**PowerShell:**
```powershell
.\build.ps1
```

**Bash (Git Bash/MSYS2):**
```bash
./build.sh
```

### Using CMake Directly

**With Ninja (fastest):**
```bash
# Configure
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc

# Build
cmake --build build

# Output: build/bin/gutpuncher.exe
```

**With MinGW Makefiles:**
```bash
# Configure
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc

# Build
cmake --build build

# Output: build/bin/gutpuncher.exe
```

### Using CMake Presets (VS Code Integration)

```bash
# Configure with preset
cmake --preset ninja-release

# Build with preset
cmake --build --preset ninja-release
```

### Debug Build
```bash
cmake -B build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc
cmake --build build-debug
```

### Clean Build
```bash
# Remove build directory
rm -rf build
# or
Remove-Item -Recurse -Force build
```

### Build Flags Explained
- `-O2`: Optimization level 2
- `-static`: Static linking (no DLL dependencies)
- `-static-libgcc`: Static link GCC libraries
- `-lpsapi`: Process API
- `-lshlwapi`: Shell API
- `-ladvapi32`: Advanced API (registry)
- `-lkernel32`: Kernel API
- `-luser32`: User API

## Usage

### Basic Usage
```bash
# Run as Administrator (REQUIRED for full functionality)
.\build\bin\gutpuncher.exe
```

Or copy to a convenient location:
```powershell
copy build\bin\gutpuncher.exe .
.\gutpuncher.exe
```

### What It Does
1. **Phase 1**: Scans and terminates malware processes (3 iterations to catch respawns)
2. **Phase 2**: Searches and destroys malware files in common locations
3. **Phase 3**: Cleans malware registry keys
4. **Phase 4**: Removes malware startup entries
5. **Phase 5**: Deletes malware scheduled tasks

### Administrator Privileges
**IMPORTANT**: Run as Administrator for full functionality!

Right-click on `gutpuncher.exe` and select "Run as administrator"

Without admin privileges, the tool will still run but may not be able to:
- Terminate protected processes
- Delete system-wide registry keys
- Remove scheduled tasks
- Delete files in protected directories

## Safety

GutPuncher is designed to be aggressive but targeted:
- Only targets processes/files matching known malware patterns
- Uses multiple validation checks before deletion
- Provides detailed logging of all actions
- Does not delete system files
- Does not require internet connection

## Output

The tool provides color-coded console output:
- 🔴 **RED**: Errors
- 🟢 **GREEN**: Successful operations
- 🟡 **YELLOW**: Warnings and detections
- 🔵 **CYAN**: Information
- ⚪ **WHITE**: General output

## Extending

To add more malware targets, edit the arrays in `gutpuncher.c`:

```c
// Add process names
const char* malware_processes[] = {
    "malware.exe",
    // ... add more
    NULL
};

// Add registry keys
const char* malware_registry_keys[] = {
    "SOFTWARE\\MalwareName",
    // ... add more
    NULL
};

// Add directories
const char* malware_directories[] = {
    "MalwareName",
    // ... add more
    NULL
};
```

## Technical Details

- **Language**: C (C99 compatible)
- **Platform**: Windows 7, 8, 10, 11 (32-bit and 64-bit)
- **Compiler**: MinGW-w64 with UCRT64
- **Linking**: Static (no runtime dependencies)
- **Runtime**: Universal C Runtime (UCRT)
- **APIs Used**: 
  - Windows Process API (tlhelp32.h, psapi.h)
  - Windows Registry API (winreg.h)
  - Windows File System API
  - Windows Shell API (shlobj.h)

## Disclaimer

This tool is designed for legitimate malware removal purposes. Use responsibly and at your own risk. Always backup important data before running aggressive cleanup tools.

## License

This is free and unencumbered software released into the public domain.

## Contributing

Found a new Comet AI variant? Submit the process names, file paths, or registry keys!

---

**Remember**: After running GutPuncher, restart your computer to complete the cleanup process.
