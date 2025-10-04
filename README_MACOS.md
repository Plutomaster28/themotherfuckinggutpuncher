# GutPuncher for macOS 🥊🍎

**Aggressive Malware Removal Tool - macOS Edition**

This is the macOS version of GutPuncher, specifically designed to hunt down and eliminate the Comet AI Browser and associated malware on Mac systems.

## 🎯 Features

- **Process Termination**: Aggressively terminates malware processes with respawn detection
- **Application Bundle Removal**: Completely removes .app bundles from /Applications
- **Cache Cleanup**: Removes all caches in ~/Library/Caches
- **Application Support Cleanup**: Deletes malware data from ~/Library/Application Support
- **LaunchAgent Removal**: Eliminates malware persistence from LaunchAgents/LaunchDaemons
- **Preference Cleanup**: Removes malware preference files
- **Universal Binary**: Works on both Intel and Apple Silicon Macs

## 🎯 Targets

### Processes
- `Comet`
- `CometBrowser`
- `Comet Helper`
- `CometAI`
- `Perplexity`
- Any process containing "comet" (case-insensitive)

### Locations
- `/Applications/Comet.app`
- `/Applications/Perplexity.app`
- `~/Library/Application Support/Comet`
- `~/Library/Application Support/Perplexity`
- `~/Library/Caches/Comet`
- `~/Library/Caches/Perplexity`
- `~/Library/Preferences/com.comet.*`
- `~/Library/Preferences/com.perplexity.*`
- `~/Library/LaunchAgents/com.comet.*`
- `/Library/LaunchAgents/com.comet.*`
- `/Library/LaunchDaemons/com.comet.*`

## 🔨 Building

### Prerequisites
- Xcode Command Line Tools
- CMake (3.15+)
- Ninja (optional but recommended)

### Install Prerequisites
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install CMake and Ninja
brew install cmake ninja
```

### Build Instructions

#### Using CMake + Ninja (Recommended)
```bash
# Configure for your Mac's architecture
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang

# Build
cmake --build build

# Output: build/bin/gutpuncher
```

#### Build Universal Binary (Intel + Apple Silicon)
```bash
# Build for x86_64
cmake -B build-x64 -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES=x86_64 \
  -DCMAKE_C_COMPILER=clang
cmake --build build-x64

# Build for ARM64
cmake -B build-arm64 -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_C_COMPILER=clang
cmake --build build-arm64

# Create universal binary
lipo -create \
  build-x64/bin/gutpuncher \
  build-arm64/bin/gutpuncher \
  -output gutpuncher-universal
  
chmod +x gutpuncher-universal
```

## 🚀 Usage

### Basic Usage
```bash
# Must run with sudo for full functionality
sudo ./build/bin/gutpuncher
```

Or if using universal binary:
```bash
sudo ./gutpuncher-universal
```

### What It Does
1. **Phase 1**: Scans and terminates malware processes (3 iterations to catch respawns)
2. **Phase 2**: Removes application bundles from /Applications
3. **Phase 3**: Cleans browser caches and application support data
4. **Phase 4**: Removes LaunchAgents and LaunchDaemons

### Root Privileges
⚠️ **IMPORTANT**: Run with `sudo` for full functionality!

Without root privileges, the tool will still run but may not be able to:
- Terminate protected processes
- Delete system-wide LaunchDaemons
- Remove files in protected directories
- Clean other user's application data

## 🛡️ Safety

GutPuncher for macOS is designed to be aggressive but targeted:
- Only targets processes/files matching known malware patterns
- Uses multiple validation checks before deletion
- Provides detailed logging of all actions
- Does not delete system files
- Does not require internet connection

## 📊 Output

The tool provides color-coded console output:
- 🔴 **RED**: Errors and warnings
- 🟢 **GREEN**: Successful operations
- 🟡 **YELLOW**: Warnings and detections
- 🔵 **CYAN**: Information
- ⚪ **WHITE**: General output

## 🔧 Extending

To add more malware targets, edit the arrays in `gutpuncher_macos.c`:

```c
// Add process names
const char* malware_processes[] = {
    "MalwareName",
    // ... add more
    NULL
};

// Add directories
const char* malware_dirs[] = {
    "/Applications/Malware.app",
    "~/Library/Application Support/Malware",
    // ... add more
    NULL
};
```

## 📝 Technical Details

- **Language**: C (C99 compatible)
- **Platform**: macOS 10.13+ (Intel & Apple Silicon)
- **Compiler**: Clang (Apple LLVM)
- **Linking**: Static where possible
- **APIs Used**: 
  - libproc (process listing)
  - POSIX file operations
  - BSD signal handling

## 🆚 Differences from Windows Version

| Feature | Windows | macOS |
|---------|---------|-------|
| Process Killing | TerminateProcess | SIGKILL |
| File Deletion | DeleteFile | unlink/rmdir |
| Registry Cleanup | ✅ Yes | ❌ N/A |
| Services | ✅ Windows Services | ✅ LaunchAgents |
| Prefetch | ✅ Yes | ❌ N/A |
| Application Bundles | ❌ N/A | ✅ .app removal |
| Startup Items | ✅ Run keys | ✅ LaunchAgents |

## ⚠️ Disclaimer

This tool is designed for legitimate malware removal purposes. Use responsibly and at your own risk. Always backup important data before running aggressive cleanup tools.

## 🤝 Contributing

Found a new Comet AI variant on macOS? Submit the process names, file paths, or bundle identifiers!

---

**Remember**: After running GutPuncher, restart your Mac to complete the cleanup process.

## 📦 Pre-built Binaries

Pre-built universal binaries are available from GitHub Actions:
- Download from the [Releases](https://github.com/Plutomaster28/themotherfuckinggutpuncher/releases) page
- Or build from source using the instructions above

## 🔗 Cross-Platform

This is the macOS version. For Windows, see the main [README.md](README.md).
