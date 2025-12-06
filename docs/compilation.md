# Compilation Guide

## Prerequisites

### Windows

- **Visual Studio** (2019 or later) with:
  - C++ Desktop development workload
  - **C++ ATL for latest build tools (x86 & x64)** - Required for DirectXShaderCompiler
- **Python 3.x** (for SCons)
- **CMake** (for building third-party libraries)

To install ATL if missing:
1. Open Visual Studio Installer
2. Click **Modify** on your installation
3. Go to **Individual components**
4. Search for and check "C++ ATL for latest v143 build tools (x86 & x64)"
5. Click **Modify** to install

### Linux

- **GCC** (9+) or **Clang** (10+)
- **Python 3.x** (for SCons)
- **CMake** (for building third-party libraries)
- **Development libraries:**
  ```bash
  # Ubuntu/Debian
  sudo apt install build-essential cmake python3 python3-venv \
      libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev \
      libxinerama-dev libxss-dev libwayland-dev libxkbcommon-dev \
      libasound2-dev libpulse-dev libudev-dev
  
  # Fedora
  sudo dnf install gcc-c++ cmake python3 \
      libX11-devel libXext-devel libXrandr-devel libXcursor-devel libXi-devel \
      libXinerama-devel libXScrnSaver-devel wayland-devel libxkbcommon-devel \
      alsa-lib-devel pulseaudio-libs-devel systemd-devel
  
  # Arch Linux
  sudo pacman -S base-devel cmake python \
      libx11 libxext libxrandr libxcursor libxi libxinerama libxss \
      wayland libxkbcommon alsa-lib libpulse
  ```

### macOS

- **Xcode Command Line Tools** or full Xcode
- **Python 3.x** (for SCons)
- **CMake** (install via Homebrew: `brew install cmake`)

## Setup

### 1. Ensure submodules are initialized

```bash
git submodule update --init --recursive
```

### 2. Create and activate a Virtual Environment

```bash
python -m venv venv
source venv/bin/activate  # On Windows use `venv\Scripts\activate`
```

### 3. Install SCons

```bash
pip install scons
```

### 4. Verify Installation

```bash
scons --version
```

## Building

### Release Build

```bash
scons
```

### Debug Build

```bash
scons debug=1
```

### Clean Build Artifacts

```bash
scons -c
```

### Parallel Build

```bash
scons -j8
```

(Replace `8` with the number of CPU cores you want to use)

## Running

After a successful build, the executables will be located in:

- **Client:** `client/client.exe` (Windows) or `client/client` (Linux/macOS)
- **Server:** `server/server.exe` (Windows) or `server/server` (Linux/macOS)

### Start the Server

```bash
./server/server
```

### Start the Client

```bash
./client/client
```

## Development

### Adding New Source Files

Simply add `.cpp` files to the appropriate `src/` directory. SCons will automatically detect and compile them.

### Adding Shared Code

1. Add headers to `shared/include/shared/`
2. Add implementations to `shared/src/`
3. Both client and server will automatically link against the updated shared library

### Adding Dependencies

1. Import a new dependency as a submodule under `/third-party/`
2. Modify `SConstruct` to include the new dependency in the build process