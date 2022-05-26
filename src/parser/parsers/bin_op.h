
#pragma once
#include "../../ast/ast.h"
#include "../../lexer/token.h"

int get_token_precedence(ast::BinOp bin_op_type);

ast::BinOp get_bin_op_from_token_type(TokenType token_type);

void init_bin_op_lookup();