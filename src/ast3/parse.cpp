

#include "parse.h"

#include <vector>

static ParseResult<AstNode*>
parse_module(Ast& ast, Cursor& cursor)
{
	std::vector<AstNode*> statements;

	while( !cursor.at_end() )
	{}

	return ast.create<AstModule>(Span(), statements);
}

ParseResult<AstNode*>
parse(Ast& ast, Cursor& cursor)
{
	return parse_module(ast, cursor);
}