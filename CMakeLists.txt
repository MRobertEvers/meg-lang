# https://llvm.org/docs/CMake.html#embedding-llvm-in-your-project
cmake_minimum_required(VERSION 3.10)

project(MegLang)
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.

set(LLVM_LINK_COMPONENTS
  Analysis
  Core
  ExecutionEngine
  InstCombine
  Object
  OrcJIT
  RuntimeDyld
  ScalarOpts
  Support
  native
)

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Now build our tools
add_executable(MegLang 
    src/main.cpp
    src/lexer/lexer.cpp
    src/lexer/lexer.h
    src/lexer/keywords.cpp
    src/lexer/keywords.h
    src/lexer/token.h
)

set_property(TARGET MegLang PROPERTY CXX_STANDARD 14)

# Find the libraries that correspond to the LLVM components
# that we wish to use
# Following the Kaleidoscope example, had to add orcjit native in Ch 4.
llvm_map_components_to_libnames(llvm_libs support core irreader object orcjit native)

# Link against LLVM libraries
target_link_libraries(MegLang ${llvm_libs})