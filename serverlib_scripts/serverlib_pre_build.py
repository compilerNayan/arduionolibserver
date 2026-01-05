# Print message immediately when script is loaded
# print("Hello from serverlib pre-build script")

# Import PlatformIO environment first (if available)
env = None
try:
    Import("env")
except NameError:
    # Not running in PlatformIO environment (e.g., running from CMake)
    # print("Note: Not running in PlatformIO environment - some features may be limited")
    # Create a mock env object for CMake builds
    class MockEnv:
        def get(self, key, default=None):
            return default
    env = MockEnv()

import sys
import os
import json
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
    
    # if project_dir:
        # print(f"\nClient project directory: {project_dir}")
    # else:
        # print("Warning: Could not determine PROJECT_DIR from environment")
    return project_dir


def get_current_library_path(project_dir=None):
    """
    Get the full path of the current library (serverlib).
    
    This function tries multiple methods to find the library:
    1. Uses PlatformIO env if available (LIB_PATH or similar)
    2. Uses __file__ if available (normal Python execution)
    3. Searches in project libraries
    
    Args:
        project_dir: Optional project directory to search in libraries list
    
    Returns:
        Path: Path object pointing to the serverlib library directory, or None if not found
    """
    # Method 1: Try PlatformIO environment (when running in PlatformIO)
    # When PlatformIO executes the script, it provides the library path
    if env:
        # Try to get library path from env - PlatformIO may set this
        lib_path = env.get("LIB_PATH", None)
        if lib_path:
            lib_path = Path(lib_path)
            if lib_path.exists() and (lib_path / "include").exists():
                return lib_path.resolve()
        
        # Also try PROJECT_LIBDEPS_DIR
        lib_dir = env.get("PROJECT_LIBDEPS_DIR", None)
        if lib_dir:
            # Look for arduionolibserver in libdeps
            libdeps_path = Path(lib_dir)
            if libdeps_path.exists():
                for platform_dir in libdeps_path.iterdir():
                    if platform_dir.is_dir():
                        for lib_dir in platform_dir.iterdir():
                            if lib_dir.is_dir() and ("serverlib" in lib_dir.name.lower() or "arduionolibserver" in lib_dir.name.lower()):
                                if (lib_dir / "include").exists() and (lib_dir / "library.json").exists():
                                    try:
                                        lib_json_path = lib_dir / "library.json"
                                        with open(lib_json_path, 'r') as f:
                                            lib_data = json.load(f)
                                            if lib_data.get("name") == "serverlib" or lib_data.get("name") == "arduionolibserver":
                                                return lib_dir.resolve()
                                    except Exception:
                                        pass
    
    # Method 2: Find by script location (when __file__ is available)
    # Use globals().get() to safely check for __file__ without raising NameError
    script_file = globals().get('__file__', None)
    if script_file:
        try:
            script_path = Path(script_file).resolve()
            scripts_dir = script_path.parent
            library_dir = scripts_dir.parent
            
            # Check if this looks like the right library (has include/ and library.json)
            if library_dir.exists() and (library_dir / "include").exists() and (library_dir / "library.json").exists():
                # Verify it's serverlib by checking library.json
                try:
                    lib_json_path = library_dir / "library.json"
                    if lib_json_path.exists():
                        with open(lib_json_path, 'r') as f:
                            lib_data = json.load(f)
                            if lib_data.get("name") == "serverlib" or lib_data.get("name") == "arduionolibserver":
                                return library_dir.resolve()
                except Exception:
                    pass
        except Exception:
            # Any error accessing __file__ - skip this method
            pass
    
    # Method 3: Search in project libraries if project_dir is provided
    if project_dir:
        libraries = find_all_libraries(project_dir)
        for lib_dir in libraries:
            lib_name = lib_dir.name
            # Check for various naming patterns (serverlib, serverlib-src, arduionolibserver, etc.)
            if "serverlib" in lib_name.lower() or "arduionolibserver" in lib_name.lower():
                # Verify by checking for include/ directory and library.json
                if (lib_dir / "include").exists() and (lib_dir / "library.json").exists():
                    try:
                        lib_json_path = lib_dir / "library.json"
                        with open(lib_json_path, 'r') as f:
                            lib_data = json.load(f)
                            if lib_data.get("name") == "serverlib" or lib_data.get("name") == "arduionolibserver":
                                return lib_dir.resolve()
                    except Exception:
                        # If can't read library.json, still return if has include/
                        return lib_dir.resolve()
    
    return None


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
        # print(f"\nSearching PlatformIO libraries in: {pio_libdeps}")
        for platform_dir in pio_libdeps.iterdir():
            if platform_dir.is_dir():
                for lib_dir in platform_dir.iterdir():
                    if lib_dir.is_dir():
                        libraries.append(lib_dir)
                        # print(f"  Found PlatformIO library: {lib_dir.name} at {lib_dir}")
    
    # Find CMake libraries in build/_deps/
    cmake_deps = project_path / "build" / "_deps"
    if cmake_deps.exists() and cmake_deps.is_dir():
        # print(f"\nSearching CMake libraries in: {cmake_deps}")
        for lib_dir in cmake_deps.iterdir():
            if lib_dir.is_dir() and not lib_dir.name.startswith("."):
                # Only include -src directories (source libraries) or directories with include/ folder
                # Skip -build and -subbuild directories
                if lib_dir.name.endswith("-src") or (lib_dir / "include").exists():
                    libraries.append(lib_dir)
                    # print(f"  Found CMake library: {lib_dir.name} at {lib_dir}")
    
    return libraries


def get_all_files(library_dir):
    """
    Get all .cpp and .h files in a library directory recursively.
    
    Args:
        library_dir: Path to the library directory
    
    Returns:
        list: List of Path objects with full absolute paths for .cpp and .h files only
    """
    files = []
    if not library_dir.exists() or not library_dir.is_dir():
        return files
    
    try:
        # Filter for only .cpp and .h files
        allowed_extensions = {'.h', '.hpp'}
        for item in library_dir.rglob("*"):
            if item.is_file() and item.suffix.lower() in allowed_extensions:
                # Resolve to full absolute path
                full_path = item.resolve()
                files.append(full_path)
    except Exception as e:
        # print(f"  Warning: Error scanning {library_dir}: {e}")
        import traceback
        # print(f"  Traceback: {traceback.format_exc()}")
    
    return files


def print_library_files(libraries):
    """
    Print all .cpp and .h files from all libraries.
    
    Args:
        libraries: List of library directory paths
    
    Returns:
        list: List of all file paths found (Path objects with full absolute paths)
    """
    all_files = []
    # print("\n" + "=" * 80)
    # print("NAYAN X LIBRARY FILES REPORT (.cpp and .h files only)")
    # print("=" * 80)
    
    if not libraries:
        # print("\nNo libraries found in the project.")
        return all_files
    
    total_files = 0
    for lib_dir in libraries:
        lib_name = lib_dir.name
        # print(f"\n{'=' * 80}")
        # print(f"Library: {lib_name}")
        # print(f"Path: {lib_dir}")
        # print(f"{'=' * 80}")
        
        files = get_all_files(lib_dir)
        total_files += len(files)
        all_files.extend(files)
        
        if files:
            # print(f"\nFound {len(files)} .cpp/.h files:")
            for file_path in sorted(files):
                # Print full absolute path (already resolved in get_all_files)
                # print(f"  {file_path}")
                pass
        else:
            # print("\nNo .cpp/.h files found in this library.")
            pass
    
    # print(f"\n{'=' * 80}")
    # print(f"Total libraries: {len(libraries)}")
    # print(f"Total .cpp/.h files across all libraries: {total_files}")
    # print("=" * 80)
    
    return all_files


def main():
    """Main function to execute the script."""
    project_dir = get_project_dir()
    if not project_dir:
        # print("Error: Could not determine project directory")
        return
    
    libraries = find_all_libraries(project_dir)
    print_library_files(libraries)
    
    # Get current library path and process @ServerImpl annotations from ALL libraries
    # print("\n" + "=" * 80)
    # print("Processing @ServerImpl annotations in all libraries...")
    # print("=" * 80)
    
    current_library_path = get_current_library_path(project_dir)
    if not current_library_path:
        # print("Error: Could not determine current library path (serverlib)")
        return
    
    # print(f"Current library path (for ServerProviderInit.h): {current_library_path}")
    
    # Get all .h/.hpp files from ALL libraries, not just the current one
    all_library_files = []
    for lib_dir in libraries:
        lib_files = get_all_files(lib_dir)
        all_library_files.extend(lib_files)
        if lib_files:
            # print(f"  Found {len(lib_files)} .h/.hpp file(s) in library: {lib_dir.name} at {lib_dir}")
            pass
        else:
            # Debug: print even if no files found, especially for libraries we expect to have files
            if "arduinodesktopserver" in lib_dir.name.lower():
                # print(f"  DEBUG: Library {lib_dir.name} at {lib_dir} - no files found")
                # print(f"    Library exists: {lib_dir.exists()}")
                # print(f"    Is directory: {lib_dir.is_dir() if lib_dir.exists() else 'N/A'}")
                include_dir = lib_dir / "include"
                # print(f"    Include dir exists: {include_dir.exists()}")
                if include_dir.exists():
                    http_file = include_dir / "HttpTcpServer.h"
                    # print(f"    HttpTcpServer.h exists: {http_file.exists()}")
                    if http_file.exists():
                        # print(f"    HttpTcpServer.h path: {http_file}")
                        pass
    
    if not all_library_files:
        # print("No .h/.hpp files found in any library.")
        return
    
    # print(f"\nTotal: Found {len(all_library_files)} .h/.hpp file(s) across all libraries")
    
    # Import and use L3_process_and_register functions
    try:
        # Add the core scripts directory to path
        # Use safe method to get script directory
        script_file = globals().get('__file__', None)
        if script_file:
            core_scripts_dir = Path(script_file).parent / "serverlib_core"
        else:
            # Fallback: use current_library_path to find scripts directory
            core_scripts_dir = current_library_path / "serverlib_scripts" / "serverlib_core"
        sys.path.insert(0, str(core_scripts_dir))
        
        from L3_process_and_register import check_and_comment_server_impl, generate_registration_code, generate_include_statements, update_server_factory_init
        
        all_registrations = []
        processed_count = 0
        commented_count = 0
        
        # Process each file from all libraries
        for file_path in all_library_files:
            # print(f"\nProcessing: {file_path}")
            result = check_and_comment_server_impl(file_path)
            
            if result['error']:
                # print(f"  Error: {result['error']}")
                continue
            
            if result['found']:
                processed_count += 1
                if result['modified']:
                    commented_count += 1
                    # print(f"  ✓ Marked {len(result['matches'])} @ServerImpl annotation(s) as processed")
                else:
                    # print(f"  ✓ Found {len(result['matches'])} @ServerImpl annotation(s) (already processed)")
                    pass
                
                # Add registrations to the list
                for match in result['matches']:
                    all_registrations.append({
                        'class_name': match['class_name'],
                        'server_impl_content': match['server_impl_content'],
                        'file_path': match.get('file_path', '')  # Include file path
                    })
                    # print(f"    - Class: {match['class_name']}, @ServerImpl: \"{match['server_impl_content']}\"")
            else:
                # print(f"  - No @ServerImpl annotation found")
                pass
        
        # print(f"\n{'=' * 80}")
        # print(f"Summary:")
        # print(f"  Files processed: {processed_count}/{len(all_library_files)}")
        # print(f"  Files with annotations processed: {commented_count}")
        # print(f"  Total registrations: {len(all_registrations)}")
        # print(f"{'=' * 80}\n")
        
        if all_registrations:
            # Generate include statements
            # print("Generating include statements...")
            include_statements = generate_include_statements(all_registrations, current_library_path)
            if include_statements:
                # print("Generated includes:")
                # print("-" * 80)
                # print(include_statements)
                # print("-" * 80)
                pass
            
            # Generate registration code
            # print("\nGenerating registration code...")
            registration_code = generate_registration_code(all_registrations)
            # print("Generated code:")
            # print("-" * 80)
            # print(registration_code)
            # print("-" * 80)
            
            # Update ServerProviderInit.h
            # print(f"\nUpdating ServerProviderInit.h...")
            result = update_server_factory_init(current_library_path, registration_code, include_statements)
            
            if result['error']:
                # print(f"Error: {result['error']}")
                pass
            elif result['success']:
                # print(f"✓ Successfully updated ServerProviderInit.h")
                pass
            else:
                # print(f"✗ Failed to update ServerProviderInit.h")
                pass
        else:
            # print("No @ServerImpl annotations found. Nothing to register.")
            pass
    
    except ImportError as e:
        # print(f"Error: Could not import L3_process_and_register functions: {e}")
        pass
    except Exception as e:
        # print(f"Error processing @ServerImpl annotations: {e}")
        pass


# Execute main function
if __name__ == "__main__":
    main()
else:
    # When imported by PlatformIO, execute immediately
    main()

