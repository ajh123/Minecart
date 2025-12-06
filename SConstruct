# History Survival - SCons Build Configuration
import platform
import subprocess

# Detect platform
is_windows = platform.system() == 'Windows'

# ============================================================================
# Version Detection from Git
# ============================================================================
def get_git_version():
    """Get version string from git tag or short commit hash."""
    try:
        # First, try to get a tag that points to the current commit
        result = subprocess.run(
            ['git', 'describe', '--tags', '--exact-match'],
            capture_output=True, text=True, cwd=Dir('.').abspath
        )
        if result.returncode == 0:
            return result.stdout.strip()

        # Fallback: just use short commit hash
        result = subprocess.run(
            ['git', 'rev-parse', '--short', 'HEAD'],
            capture_output=True, text=True, cwd=Dir('.').abspath
        )
        if result.returncode == 0:
            return result.stdout.strip()
    except Exception as e:
        print(f"Warning: Could not determine git version: {e}")

    return 'unknown'

game_version = get_git_version()
print(f"Building version: {game_version}")

# Create base environment
env = Environment()

# Set C++ standard
if is_windows:
    env.Append(CXXFLAGS=['/std:c++20', '/EHsc', '/W4', '/utf-8'])
else:
    env.Append(CXXFLAGS=['-std=c++20', '-Wall', '-Wextra', '-Wpedantic'])

# Debug/Release configuration
debug = ARGUMENTS.get('debug', 0)
if int(debug):
    if is_windows:
        env.Append(CXXFLAGS=['/Od', '/Zi'])
        env.Append(LINKFLAGS=['/DEBUG'])
    else:
        env.Append(CXXFLAGS=['-g', '-O0'])
    env.Append(CPPDEFINES=['DEBUG'])
    build_type = 'Debug'
else:
    if is_windows:
        env.Append(CXXFLAGS=['/O2'])
    else:
        env.Append(CXXFLAGS=['-O2'])
    env.Append(CPPDEFINES=['NDEBUG'])
    build_type = 'Release'

# Add PACKAGE_VERSION define
env.Append(CPPDEFINES=[('PACKAGE_VERSION', f'\\"{game_version}\\"')])

# ============================================================================
# Third-party libraries (SDL3, ImGui)
# ============================================================================
SConscript('third-party/SConstruct', exports=['env', 'build_type', 'is_windows'])

# ============================================================================
# Build minecart library first
minecart_lib = SConscript('minecart/SConscript', exports='env')
