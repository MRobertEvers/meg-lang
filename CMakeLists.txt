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
    src/ast2/bin_op.cpp
    src/ast2/ParseTrail.cpp
    src/ast2/ParseResult.cpp
    src/ast2/AstNode.cpp
    src/ast2/AstGen.cpp
    src/ast2/AstCasts.cpp
    src/ast2/AstTags.h
    src/ast2/Ast.cpp
    src/sema2/Scope.cpp
    src/sema2/SemaGen.cpp
    src/sema2/Sema2.cpp
    src/sema2/SemaResult.cpp
    src/sema2/type/Type.cpp
    src/sema2/type/FunctionTypeInfo.cpp
    src/sema2/type/StructTypeInfo.cpp
    src/sema2/Types.cpp
    src/sema2/TypeInstance.cpp
    src/sema2/TypedMember.cpp
    src/sema2/SemaTag.cpp
    src/codegen2/Codegen.cpp
    src/codegen2/CGResult.cpp
    src/codegen2/CGExpr.cpp
    src/codegen2/Scope.cpp
    src/codegen2/LValue.cpp
    src/codegen2/Codegen/codegen_return.cpp
    src/codegen2/Codegen/codegen_function.cpp
    src/codegen2/Codegen/codegen_addressof.cpp
    src/codegen2/Codegen/codegen_deref.cpp
    src/codegen2/Codegen/codegen_string_literal.cpp
    src/codegen2/Codegen/codegen_assign.cpp
    src/codegen2/Codegen/codegen_statement.cpp
    src/codegen2/Codegen/codegen_call.cpp
    src/codegen2/Codegen/codegen_fn_sig_info.cpp
    src/codegen2/Codegen/lookup.cpp
    src/codegen2/Codegen/operand.cpp
    src/codegen2/Codegen/RValue.cpp
    src/codegen2/Codegen/LLVMAddress.cpp
    src/codegen2/Codegen/LLVMFnInfo.cpp
    src/codegen2/Codegen/LLVMFnInfoBuilder.cpp
    src/codegen2/Codegen/LLVMFnSigInfo.cpp
    src/codegen2/Codegen/LLVMFnSigInfoBuilder.cpp
    src/codegen2/Codegen/CGNotImpl.cpp
)

# # Now build our tools
# add_executable(sushi 
#     src/sushi_main.cpp
#     src/lexer/Lexer.cpp
#     src/lexer/Lexer.h
#     src/lexer/keywords.cpp
#     src/lexer/keywords.h
#     src/lexer/token.h
#     src/lexer/TokenCursor.h
#     src/lexer/TokenCursor.cpp
#     src/common/OwnPtr.h
#     src/common/Vec.h
#     src/common/String.h
#     src/format/FormatParser.h
#     src/format/FormatParser.cpp
#     src/format/NodeSpan.h
#     src/format/NodeSpan.cpp
#     src/format/pretty_print_ast.h
#     src/format/pretty_print_ast.cpp
#     src/format/reverse.h
#     src/format/utils.h
#     src/format/utils.cpp
#     src/codegen/Codegen.h
#     src/codegen/Codegen.cpp
#     src/parser/ParseResult.h
#     src/parser/ParseResult.cpp
#     src/parser/parsers/bin_op.h
#     src/parser/parsers/bin_op.cpp
#     src/parser/parsers/Parser.h
#     src/parser/parsers/Parser.cpp
#     src/parser/parsers/ParseTrail.h
#     src/parser/parsers/ParseTrail.cpp
#     src/sema/Sema.cpp
#     src/sema/Sema.h
#     src/sema/SemaResult.h
#     src/sema/SemaResult.cpp
#     src/sema/Type.h
#     src/sema/Type.cpp
#     src/ast/IAstNode.h
#     src/ast/expressions/Expression.h
#     src/ast/expressions/BinOp.h
#     src/ast/expressions/Identifier.h
#     src/ast/expressions/MemberReference.h
#     src/ast/expressions/Number.h
#     src/ast/statements/Let.h
#     src/ast/statements/Statement.h
#     src/ast/statements/If.h
#     src/ast/statements/For.h
#     src/ast/statements/Block.h
#     src/ast/statements/Function.h
#     src/ast/statements/Prototype.h
#     src/ast/statements/Return.h
#     src/ast/statements/Struct.h
# )

set_property(TARGET sushi PROPERTY CXX_STANDARD 17)

# add_executable(sushi_format 
#     src/sushi_format.cpp
#     src/lexer/Lexer.cpp
#     src/lexer/Lexer.h
#     src/lexer/keywords.cpp
#     src/lexer/keywords.h
#     src/lexer/token.h
#     src/lexer/TokenCursor.h
#     src/lexer/TokenCursor.cpp
#     src/common/OwnPtr.h
#     src/common/Vec.h
#     src/common/String.h
#     src/format/FormatParser.h
#     src/format/FormatParser.cpp
#     src/format/NodeSpan.h
#     src/format/NodeSpan.cpp
#     src/format/pretty_print_ast.h
#     src/format/pretty_print_ast.cpp
#     src/format/reverse.h
#     src/format/utils.h
#     src/format/utils.cpp
#     src/parser/ParseResult.h
#     src/parser/ParseResult.cpp
#     src/parser/parsers/bin_op.h
#     src/parser/parsers/bin_op.cpp
#     src/parser/parsers/Parser.h
#     src/parser/parsers/Parser.cpp
#     src/parser/parsers/ParseTrail.h
#     src/parser/parsers/ParseTrail.cpp
#     src/ast/IAstNode.h
#     src/ast/expressions/Expression.h
#     src/ast/expressions/BinOp.h
#     src/ast/expressions/Identifier.h
#     src/ast/expressions/MemberReference.h
#     src/ast/expressions/Number.h
#     src/ast/statements/Let.h
#     src/ast/statements/Statement.h
#     src/ast/statements/If.h
#     src/ast/statements/For.h
#     src/ast/statements/Block.h
#     src/ast/statements/Function.h
#     src/ast/statements/Prototype.h
#     src/ast/statements/Return.h
#     src/ast/statements/Struct.h
# )

# set_property(TARGET sushi_format PROPERTY CXX_STANDARD 17)

# Find the libraries that correspond to the LLVM components
# that we wish to use
# Following the Kaleidoscope example, had to add orcjit native in Ch 4.
llvm_map_components_to_libnames(llvm_libs support core irreader object orcjit native)

# Link against LLVM libraries
target_link_libraries(sushi ${llvm_libs})