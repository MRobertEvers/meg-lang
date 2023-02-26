#include "parse_namespace.h"

#include "../AstGen.h"

using namespace ast;

ParseResult<AstNode*>
ast::parse_namespace(AstGen& astgen)
{
	auto trail = astgen.get_parse_trail();

	auto consume_tok = astgen.cursor.consume(TokenType::namespace_keyword);
	if( !consume_tok.ok() )
		return ParseError("Expected 'namespace'", consume_tok.as());

	auto identifier = astgen.parse_identifier();
	if( !identifier.ok() )
		return identifier;

	auto name_node = identifier.unwrap();

	consume_tok = astgen.cursor.consume(TokenType::open_curly);
	if( !consume_tok.ok() )
		return ParseError("Expected block, '{'.", consume_tok.as());

	auto nodes = astgen.ast.create_list();

	while( astgen.cursor.peek().type != TokenType::close_curly )
	{
		auto item = astgen.parse_module_top_level_item();
		if( !item.ok() )
		{
			return item;
		}
		nodes->append(item.unwrap());
	}

	consume_tok = astgen.cursor.consume(TokenType::close_curly);
	if( !consume_tok.ok() )
		return ParseError("Expected block, '}'.", consume_tok.as());

	return astgen.ast.Namespace(trail.mark(), name_node, nodes);
}