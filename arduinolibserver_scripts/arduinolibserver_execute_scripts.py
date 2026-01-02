"""
Script to execute client file processing.
This script finds and prints all files from all libraries in the client project.
"""

import sys
import os
from pathlib import Path

# Import debug utility
try:
    from debug_utils import debug_print
except ImportError:
    # Fallback if debug_utils not found - create a no-op function
    def debug_print(*args, **kwargs):
        pass



def get_project_dir():
    """
    Get the project directory from environment or current working directory.
    
    Returns:
        str: Path to the project directory
    """
    project_dir = os.environ.get("PROJECT_DIR", None)
    if not project_dir:
        project_dir = os.environ.get("CMAKE_PROJECT_DIR", None)
    if not project_dir:
        project_dir = os.getcwd()
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
        for platform_dir in pio_libdeps.iterdir():
            if platform_dir.is_dir():
                for lib_dir in platform_dir.iterdir():
                    if lib_dir.is_dir():
                        libraries.append(lib_dir)
    
    # Find CMake libraries in build/_deps/
    cmake_deps = project_path / "build" / "_deps"
    if cmake_deps.exists() and cmake_deps.is_dir():
        for lib_dir in cmake_deps.iterdir():
            if lib_dir.is_dir() and not lib_dir.name.startswith("."):
                libraries.append(lib_dir)
    
    return libraries


def get_all_files(library_dir):
    """
    Get all files in a library directory recursively.
    
    Args:
        library_dir: Path to the library directory
    
    Returns:
        list: List of Path objects for all files
    """
    files = []
    if not library_dir.exists() or not library_dir.is_dir():
        return files
    
    try:
        for item in library_dir.rglob("*"):
            if item.is_file():
                files.append(item)
    except Exception as e:
        debug_print(f"  Warning: Error scanning {library_dir}: {e}")
    
    return files


def execute_scripts(project_dir, library_dir):
    """
    Execute the scripts to process and print all library files.
    
    Args:
        project_dir: Path to the client project root (where platformio.ini is)
        library_dir: Path to the library directory (not used in this implementation)
    """
    debug_print(f"\nproject_dir: {project_dir}")
    debug_print(f"library_dir: {library_dir}")
    
    if not project_dir:
        debug_print("Error: Project directory not provided")
        return
    
    libraries = find_all_libraries(project_dir)
    
    debug_print("\n" + "=" * 80)
    debug_print("LIBRARY FILES REPORT")
    debug_print("=" * 80)
    
    if not libraries:
        debug_print("\nNo libraries found in the project.")
        return
    
    total_files = 0
    for lib_dir in libraries:
        lib_name = lib_dir.name
        debug_print(f"\n{'=' * 80}")
        debug_print(f"Library: {lib_name}")
        debug_print(f"Path: {lib_dir}")
        debug_print(f"{'=' * 80}")
        
        files = get_all_files(lib_dir)
        total_files += len(files)
        
        if files:
            debug_print(f"\nFound {len(files)} files:")
            for file_path in sorted(files):
                # Print relative path from library root
                try:
                    rel_path = file_path.relative_to(lib_dir)
                    debug_print(f"  {rel_path}")
                except ValueError:
                    debug_print(f"  {file_path}")
        else:
            debug_print("\nNo files found in this library.")
    
    debug_print(f"\n{'=' * 80}")
    debug_print(f"Total libraries: {len(libraries)}")
    debug_print(f"Total files across all libraries: {total_files}")
    debug_print("=" * 80)

