cmake_minimum_required(VERSION 3.14)
project(arduionolibserver VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch dependencies
include(FetchContent)

# Fetch arduinolib0 for StandardDefines.h
FetchContent_Declare(
    arduinolib0
    GIT_REPOSITORY https://github.com/compilerNayan/arduinolib0.git
    GIT_TAG        main  # Use the main branch
)
FetchContent_MakeAvailable(arduinolib0)

# Run pre-build script during configuration (runs even if client creates own library)
# This will run whenever this CMakeLists.txt is processed
# Note: arduinolib0 is now available since FetchContent_MakeAvailable was called above
find_program(PYTHON_EXECUTABLE python3 python)
if(PYTHON_EXECUTABLE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/arduinolibserver_scripts/arduinolibserver_pre_build.py")
    # Get the client project directory (CMAKE_SOURCE_DIR is the top-level project source directory)
    # When this library is included via FetchContent, CMAKE_SOURCE_DIR points to the client project
    set(CLIENT_PROJECT_DIR ${CMAKE_SOURCE_DIR})
    
    # Set environment variable for the script
    set(ENV{CMAKE_PROJECT_DIR} ${CLIENT_PROJECT_DIR})
    
    execute_process(
        COMMAND ${PYTHON_EXECUTABLE} 
            "${CMAKE_CURRENT_SOURCE_DIR}/arduinolibserver_scripts/arduinolibserver_pre_build.py"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE PRE_BUILD_RESULT
        OUTPUT_VARIABLE PRE_BUILD_OUTPUT
        ERROR_VARIABLE PRE_BUILD_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
    )
    if(PRE_BUILD_OUTPUT)
        message(STATUS "arduinolibserver pre-build output:\n${PRE_BUILD_OUTPUT}")
    endif()
    if(PRE_BUILD_ERROR)
        message(STATUS "arduinolibserver pre-build errors (expected in non-PlatformIO environment):\n${PRE_BUILD_ERROR}")
    endif()
    if(PRE_BUILD_RESULT AND NOT PRE_BUILD_RESULT EQUAL "0")
        message(STATUS "arduinolibserver pre-build script exited with code ${PRE_BUILD_RESULT} (this is expected in non-PlatformIO CMake builds)")
    endif()
endif()

# Create header-only library (INTERFACE library)
add_library(arduionolibserver INTERFACE)

# Set target properties
set_target_properties(arduionolibserver PROPERTIES
    VERSION ${PROJECT_VERSION}
)

# Set include directories
target_include_directories(arduionolibserver INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Add arduinolib0 include directory (for StandardDefines.h)
# Use BUILD_INTERFACE generator expression to properly handle FetchContent paths
FetchContent_GetProperties(arduinolib0)
if(arduinolib0_SOURCE_DIR)
    target_include_directories(arduionolibserver INTERFACE
        $<BUILD_INTERFACE:${arduinolib0_SOURCE_DIR}/include>
    )
endif()

# Link arduinolib0 if it provides a target
if(TARGET arduinolib0::arduinolib0)
    target_link_libraries(arduionolibserver INTERFACE arduinolib0::arduinolib0)
elseif(TARGET arduinolib0)
    target_link_libraries(arduionolibserver INTERFACE arduinolib0)
endif()

# Add pre-build script execution
# Note: This script requires PlatformIO environment to run properly
find_program(PYTHON_EXECUTABLE python3 python REQUIRED)

# Create a custom target that runs the pre-build script
# Set the project directory environment variable for the script
if(CMAKE_SOURCE_DIR)
    set(CLIENT_PROJECT_DIR ${CMAKE_SOURCE_DIR})
else()
    # Fallback if CMAKE_SOURCE_DIR is not available
    get_filename_component(CLIENT_PROJECT_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
endif()

add_custom_target(arduinolibserver_pre_build
    COMMAND ${CMAKE_COMMAND} -E env "CMAKE_PROJECT_DIR=${CLIENT_PROJECT_DIR}"
        ${PYTHON_EXECUTABLE} 
        "${CMAKE_CURRENT_SOURCE_DIR}/arduinolibserver_scripts/arduinolibserver_pre_build.py"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running arduinolibserver pre-build script"
    VERBATIM
)

# Make the library depend on the pre-build step
add_dependencies(arduionolibserver arduinolibserver_pre_build)

# Optional: Set up installation
include(GNUInstallDirs)

install(TARGETS arduionolibserver
    EXPORT arduionolibserverTargets
)

install(EXPORT arduionolibserverTargets
    FILE arduionolibserverTargets.cmake
    NAMESPACE arduionolibserver::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/arduionolibserver
)

