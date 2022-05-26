# https://llvm.org/docs/CMake.html#embedding-llvm-in-your-project
cmake_minimum_required(VERSION 3.10)

project(Sushi)
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

include_directories(src)

# Now build our tools
add_executable(sushi 
    src/main.cpp
    src/lexer/Lexer.cpp
    src/lexer/Lexer.h
    src/lexer/keywords.cpp
    src/lexer/keywords.h
    src/lexer/token.h
    src/lexer/TokenCursor.h
    src/lexer/TokenCursor.cpp
    src/common/OwnPtr.h
    src/common/Vec.h
    src/common/String.h
    src/format/FormatParser.h
    src/format/FormatParser.cpp
    src/format/NodeSpan.h
    src/format/NodeSpan.cpp
    src/format/pretty_print_ast.h
    src/format/pretty_print_ast.cpp
    src/format/reverse.h
    src/format/utils.h
    src/format/utils.cpp
    src/codegen/Codegen.h
    src/codegen/Codegen.cpp
    src/parser/ParseResult.h
    src/parser/ParseResult.cpp
    src/parser/parsers/bin_op.h
    src/parser/parsers/bin_op.cpp
    src/parser/parsers/Parser.h
    src/parser/parsers/Parser.cpp
    src/parser/parsers/ParseTrail.h
    src/parser/parsers/ParseTrail.cpp
    src/sema/Sema.cpp
    src/sema/Sema.h
    src/sema/SemaResult.h
    src/sema/SemaResult.cpp
    src/sema/Type.h
    src/sema/Type.cpp
    src/ast/IAstNode.h
    src/ast/expressions/Expression.h
    src/ast/expressions/BinOp.h
    src/ast/expressions/Identifier.h
    src/ast/expressions/MemberReference.h
    src/ast/expressions/Number.h
    src/ast/statements/Let.h
    src/ast/statements/Statement.h
    src/ast/statements/If.h
    src/ast/statements/For.h
    src/ast/statements/Block.h
    src/ast/statements/Function.h
    src/ast/statements/Prototype.h
    src/ast/statements/Return.h
    src/ast/statements/Struct.h
)

set_property(TARGET sushi PROPERTY CXX_STANDARD 17)

# Find the libraries that correspond to the LLVM components
# that we wish to use
# Following the Kaleidoscope example, had to add orcjit native in Ch 4.
llvm_map_components_to_libnames(llvm_libs support core irreader object orcjit native)

# Link against LLVM libraries
target_link_libraries(sushi ${llvm_libs})