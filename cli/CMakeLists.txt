SET(SOURCES ${PROJECT_SOURCE_DIR}/cli/fiction.cpp)

# Include configuration file
include_directories(${PROJECT_BINARY_DIR}/include/)

# Build executable
add_executable(fiction ${SOURCES})

# Link against Z3, alice, lorina, and pybind11
target_link_libraries(fiction PRIVATE project_options project_warnings fictionlib alice mockturtle)
