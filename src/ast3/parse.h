#pragma once

#include "ParseResult.h"
#include "ast3/Ast.h"
#include "ast3/AstNode.h"
#include "lex3/Cursor.h"

ParseResult<AstNode*> parse(Ast& ast, Cursor& cursor);