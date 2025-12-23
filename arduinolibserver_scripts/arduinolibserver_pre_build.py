# Print message immediately when script is loaded
print("Hello from arduinolibserver pre-build script")

# Import PlatformIO environment first (if available)
env = None
try:
    Import("env")
except NameError:
    # Not running in PlatformIO environment (e.g., running from CMake)
    print("Note: Not running in PlatformIO environment - some features may be limited")
    # Create a mock env object for CMake builds
    class MockEnv:
        def get(self, key, default=None):
            return default
    env = MockEnv()

import sys
import os
from pathlib import Path


def get_project_dir():
    """
    Get the project directory from PlatformIO environment or CMake environment.
    
    Returns:
        str: Path to the project directory, or None if not found
    """
    # Try PlatformIO environment first
    project_dir = None
    if env:
        project_dir = env.get("PROJECT_DIR", None)
    
    # If not found, try CMake environment variable
    if not project_dir:
        project_dir = os.environ.get("CMAKE_PROJECT_DIR", None)
    
    # If still not found, try current working directory
    if not project_dir:
        project_dir = os.getcwd()
    
    if project_dir:
        print(f"\nClient project directory: {project_dir}")
    else:
        print("Warning: Could not determine PROJECT_DIR from environment")
    return project_dir


def find_all_libraries(project_dir):
    """
    Find all library directories in the project (both PlatformIO and CMake).
    
    Args:
        project_dir: Path to the project root directory
    
    Returns:
        list: List of Path objects pointing to library directories
    """
    libraries = []
    project_path = Path(project_dir)
    
    # Find PlatformIO libraries in .pio/libdeps/
    pio_libdeps = project_path / ".pio" / "libdeps"
    if pio_libdeps.exists() and pio_libdeps.is_dir():
        print(f"\nSearching PlatformIO libraries in: {pio_libdeps}")
        for platform_dir in pio_libdeps.iterdir():
            if platform_dir.is_dir():
                for lib_dir in platform_dir.iterdir():
                    if lib_dir.is_dir():
                        libraries.append(lib_dir)
                        print(f"  Found PlatformIO library: {lib_dir.name} at {lib_dir}")
    
    # Find CMake libraries in build/_deps/
    cmake_deps = project_path / "build" / "_deps"
    if cmake_deps.exists() and cmake_deps.is_dir():
        print(f"\nSearching CMake libraries in: {cmake_deps}")
        for lib_dir in cmake_deps.iterdir():
            if lib_dir.is_dir() and not lib_dir.name.startswith("."):
                libraries.append(lib_dir)
                print(f"  Found CMake library: {lib_dir.name} at {lib_dir}")
    
    return libraries


def get_all_files(library_dir):
    """
    Get all .cpp and .h files in a library directory recursively.
    
    Args:
        library_dir: Path to the library directory
    
    Returns:
        list: List of Path objects for .cpp and .h files only
    """
    files = []
    if not library_dir.exists() or not library_dir.is_dir():
        return files
    
    try:
        # Filter for only .cpp and .h files
        allowed_extensions = {'.cpp', '.h', '.hpp', '.c', '.cc', '.cxx', '.hxx'}
        for item in library_dir.rglob("*"):
            if item.is_file() and item.suffix.lower() in allowed_extensions:
                files.append(item)
    except Exception as e:
        print(f"  Warning: Error scanning {library_dir}: {e}")
    
    return files


def print_library_files(libraries):
    """
    Print all .cpp and .h files from all libraries.
    
    Args:
        libraries: List of library directory paths
    """
    print("\n" + "=" * 80)
    print("NAYAN X LIBRARY FILES REPORT (.cpp and .h files only)")
    print("=" * 80)
    
    if not libraries:
        print("\nNo libraries found in the project.")
        return
    
    total_files = 0
    for lib_dir in libraries:
        lib_name = lib_dir.name
        print(f"\n{'=' * 80}")
        print(f"Library: {lib_name}")
        print(f"Path: {lib_dir}")
        print(f"{'=' * 80}")
        
        files = get_all_files(lib_dir)
        total_files += len(files)
        
        if files:
            print(f"\nFound {len(files)} .cpp/.h files:")
            for file_path in sorted(files):
                # Print full absolute path
                full_path = file_path.resolve()
                print(f"  {full_path}")
        else:
            print("\nNo .cpp/.h files found in this library.")
    
    print(f"\n{'=' * 80}")
    print(f"Total libraries: {len(libraries)}")
    print(f"Total .cpp/.h files across all libraries: {total_files}")
    print("=" * 80)


def main():
    """Main function to execute the script."""
    project_dir = get_project_dir()
    if not project_dir:
        print("Error: Could not determine project directory")
        return
    
    libraries = find_all_libraries(project_dir)
    print_library_files(libraries)


# Execute main function
if __name__ == "__main__":
    main()
else:
    # When imported by PlatformIO, execute immediately
    main()

