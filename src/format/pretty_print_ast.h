#pragma once

#include "ast/IAstNode.h"
#include "common/Vec.h"
#include "lexer/token.h"

void pretty_print_ast(Vec<Token> const& source, ast::IAstNode const* node);