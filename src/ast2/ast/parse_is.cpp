#include "parse_is.h"

#include "../Ast.h"
#include "../AstGen.h"
#include "../bin_op.h"
#include "parse_common.h"
#include "parse_struct.h"

#include <string>

using namespace ast;

ParseResult<AstNode*>
ast::parse_is(AstGen& astgen)
{
	auto trail = astgen.get_parse_trail();

	auto lhs = astgen.parse_expr();
	if( !lhs.ok() )
		return lhs;

	auto consume_tok = astgen.cursor.consume(TokenType::is);
	if( !consume_tok.ok() )
		return ParseError("Expected 'is'.", consume_tok.as());

	auto type_decl = astgen.parse_type_decl(false);
	if( !type_decl.ok() )
		return type_decl;

	return astgen.ast.Is(trail.mark(), lhs.unwrap(), type_decl.unwrap());
}