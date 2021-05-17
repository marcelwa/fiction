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
if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
  if(ENABLE_BUILD_WITH_TIME_TRACE)
    target_compile_options(project_options INTERFACE -ftime-trace)
  endif()
endif()

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
    option(ENABLE_PROGRESS_BARS "Enable animated progress bars in command line" ON)
    if (ENABLE_PROGRESS_BARS)
        add_definitions(-DPROGRESS_BARS)
    endif ()
endif()

# Include header files
include_directories(src/algo/ src/io/ src/io/cmd/ src/tech/ src/topo/ src/util/)

# Find source files
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

# Add configuration file
configure_file(src/util/version_info.h.in util/version_info.h)
# Include configuration file
include_directories(${PROJECT_BINARY_DIR}/util/)

# Testing
option(FICTION_TEST "Build tests" OFF)

# Include tests
if (FICTION_TEST)
    enable_testing()
    message(STATUS "Building tests")
    add_subdirectory(test)
endif ()

# Custom install prefix for libraries
set(LIB_PREFIX ${CMAKE_SOURCE_DIR}/libs)
add_subdirectory(${LIB_PREFIX})

# Require Boost libraries
find_package(Boost COMPONENTS system filesystem)
if (Boost_FOUND)
	include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
	link_directories(${Boost_LIBRARY_DIRS})
else ()
	message(STATUS "Please configure Boost_CUSTOM_INCLUDE_DIRS and Boost_CUSTOM_LIBRARY_DIRS manually.")
	set(Boost_CUSTOM_INCLUDE_DIRS "" CACHE PATH "Boost include path")
	set(Boost_CUSTOM_LIBRARY_DIRS "" CACHE PATH "Boost library path")
	include_directories(SYSTEM ${Boost_CUSTOM_INCLUDE_DIRS})
	link_directories(${Boost_CUSTOM_LIBRARY_DIRS})
endif ()

# Add option to compile libraries with verbose output
option(BUILD_LIBS_VERBOSE "Output status and warnings during library build" OFF)
# Use /dev/null to silence libraries. Not elegant but efficient
if (NOT BUILD_LIBS_VERBOSE)
    set(LIB_OUTPUT_CHANNEL > /dev/null)
endif ()

# Set up a directory for Z3 solver
set(Z3_DIR ${PROJECT_BINARY_DIR}/z3)

if (UNIX)
    # Require Python interpreter
    if (${CMAKE_VERSION} VERSION_LESS "3.12.0")
        find_package(PythonInterp)  # deprecated since CMake 3.12, but works far better
        set(Z3_PYTHON ${PYTHON_EXECUTABLE})
    else ()
        find_package(Python3 COMPONENTS Interpreter Development)
        set(Z3_PYTHON ${Python3_EXECUTABLE})
    endif ()

    # Add option to build Z3 as a static library
    option(BUILD_Z3_STATIC_LIB "Build Z3 solver as a static library to link against fiction" OFF)
    if (BUILD_Z3_STATIC_LIB)
        set(Z3_LIB_FLAG --staticlib)
        set(Z3_LINK_TARGET libz3.a)
    else ()
        if (APPLE)
            set(Z3_LINK_TARGET libz3.dylib)
        else ()
            set(Z3_LINK_TARGET libz3.so)
        endif ()
    endif ()

    # include and library paths
    set(Z3_INCLUDE_DIR ${Z3_DIR}/include)
    set(Z3_LIB_DIR ${Z3_DIR}/lib)

    # Build and locally install Z3
    add_custom_command(
            OUTPUT ${Z3_LIB_DIR}/${Z3_LINK_TARGET}
            PRE_BUILD
            COMMAND ${Z3_PYTHON} scripts/mk_make.py -s --prefix=${Z3_DIR} ${Z3_LIB_FLAG} ${LIB_OUTPUT_CHANNEL}
            COMMAND $(MAKE) -C build ${LIB_OUTPUT_CHANNEL}
            COMMAND $(MAKE) -C build install ${LIB_OUTPUT_CHANNEL}
            WORKING_DIRECTORY ${LIB_PREFIX}/z3/)

    # Make sure Z3's custom build commands are actually being executed
    add_custom_target(z3
            ALL
            DEPENDS ${Z3_LIB_DIR}/${Z3_LINK_TARGET})

elseif (WIN32)
    set(Z3_LINK_TARGET libz3.lib)
    set(Z3_INCLUDE_DIR "${LIB_PREFIX}/z3/src/api" CACHE PATH "Path to Z3's custom include directory")
    set(Z3_LIB_DIR     "${LIB_PREFIX}/z3/build" CACHE PATH "Path to Z3's custom library directory")
    include_directories(SYSTEM ${Z3_INCLUDE_DIR}/c++)
endif ()

# Include Z3
include_directories(SYSTEM ${Z3_INCLUDE_DIR}/)

# Include cppitertools
include_directories(SYSTEM ${LIB_PREFIX}/cppitertools/)

# Build executable
add_executable(fiction ${SOURCES})
if (UNIX)
    add_dependencies(fiction z3)
endif ()

# Link against Boost, Z3, alice, lorina, and pybind11
target_link_libraries(fiction PRIVATE project_options project_warnings ${Boost_FILESYSTEM_LIBRARIES} ${Boost_SYSTEM_LIBRARIES}
                                      ${Z3_LIB_DIR}/${Z3_LINK_TARGET} alice mockturtle pybind11::embed ${Python3_LIBRARIES})
