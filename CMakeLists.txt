# Large parts of this build system are based on Jason Turner's C++ Starter Project (https://github.com/lefticus/cpp_starter_project)

cmake_minimum_required(VERSION 3.15)
project(fiction
        LANGUAGES CXX
        VERSION 0.4.0)

include(cmake/StandardProjectSettings.cmake)
include(cmake/PreventInSourceBuilds.cmake)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Interface library to set project options
add_library(project_options INTERFACE)

# Option to enable time tracing with clang
if (CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
    if (ENABLE_BUILD_WITH_TIME_TRACE)
        target_compile_options(project_options INTERFACE -ftime-trace)
    endif ()
endif ()

# Use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# Enable cache system
include(cmake/Cache.cmake)

# Standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# Sanitizer options if supported by compiler
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# Allow for static analysis options
include(cmake/StaticAnalyzers.cmake)

# Enable progress bars
if (NOT WIN32)
    option(FICTION_PROGRESS_BARS "Enable animated progress bars in command line" ON)
    if (FICTION_PROGRESS_BARS)
        add_definitions(-DPROGRESS_BARS)
    endif ()
endif ()

# Include header files
add_subdirectory(${CMAKE_SOURCE_DIR}/include)

# CLI
option(FICTION_CLI "Build fiction CLI" ON)
if (FICTION_CLI)
    message(STATUS "Building fiction CLI")
    add_subdirectory(${CMAKE_SOURCE_DIR}/cli)
endif ()

# Testing
option(FICTION_TEST "Build fiction tests" OFF)
if (FICTION_TEST)
    enable_testing()
    message(STATUS "Building fiction tests")
    add_subdirectory(test)
endif ()

# Include libraries
add_subdirectory(${CMAKE_SOURCE_DIR}/libs)
