"""
Script to execute client file processing.
This script finds and prints all files from all libraries in the client project.
"""

import sys
import os
from pathlib import Path


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
        # print(f"  Warning: Error scanning {library_dir}: {e}")
        pass
    
    return files


def execute_scripts(project_dir, library_dir):
    """
    Execute the scripts to process and print all library files.
    
    Args:
        project_dir: Path to the client project root (where platformio.ini is)
        library_dir: Path to the library directory (not used in this implementation)
    """
    # print(f"\nproject_dir: {project_dir}")
    # print(f"library_dir: {library_dir}")
    
    if not project_dir:
        # print("Error: Project directory not provided")
        return
    
    libraries = find_all_libraries(project_dir)
    
    # print("\n" + "=" * 80)
    # print("LIBRARY FILES REPORT")
    # print("=" * 80)
    
    if not libraries:
        # print("\nNo libraries found in the project.")
        return
    
    total_files = 0
    for lib_dir in libraries:
        lib_name = lib_dir.name
        # print(f"\n{'=' * 80}")
        # print(f"Library: {lib_name}")
        # print(f"Path: {lib_dir}")
        # print(f"{'=' * 80}")
        
        files = get_all_files(lib_dir)
        total_files += len(files)
        
        if files:
            # print(f"\nFound {len(files)} files:")
            for file_path in sorted(files):
                # Print relative path from library root
                try:
                    rel_path = file_path.relative_to(lib_dir)
                    # print(f"  {rel_path}")
                except ValueError:
                    # print(f"  {file_path}")
                    pass
        else:
            # print("\nNo files found in this library.")
            pass
    
    # print(f"\n{'=' * 80}")
    # print(f"Total libraries: {len(libraries)}")
    # print(f"Total files across all libraries: {total_files}")
    # print("=" * 80)

