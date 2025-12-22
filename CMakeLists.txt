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

