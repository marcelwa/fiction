# Include alice
add_subdirectory(alice/)

# Include mockturtle
add_subdirectory(mockturtle/)


## Enable Mugen
#option(ENABLE_MUGEN "Enable the usage of Mugen, a Python3 library by Winston Haaswijk for FCN one-pass synthesis, and its dependencies" ON)
#if (ENABLE_MUGEN)
#    add_definitions(-DMUGEN)
#endif ()
#
## Build glucose-syrup-4.1-parallel if Mugen is enabled
#if (ENABLE_MUGEN)
#    add_custom_command(
#            OUTPUT ${PROJECT_BINARY_DIR}/glucose-syrup
#            PRE_BUILD
#            COMMAND make ${LIB_OUTPUT_CHANNEL}
#            COMMAND mv glucose-syrup ${PROJECT_BINARY_DIR}/glucose-syrup ${LIB_OUTPUT_CHANNEL}
#            COMMAND make clean ${LIB_OUTPUT_CHANNEL}
#            WORKING_DIRECTORY ${LIB_PREFIX}/mugen/glucose-syrup-4.1/parallel/)
#
#    # Make sure glucose's custom build commands are actually being executed
#    add_custom_target(glucose_syrup
#            ALL
#            DEPENDS ${PROJECT_BINARY_DIR}/glucose-syrup)
#endif ()
