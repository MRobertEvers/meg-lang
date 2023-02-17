#include "parse_common.h"

using namespace ast;

AstNode*
ast::to_value_identifier(Ast& ast, ConsumeResult const& tok_res, Span span)
{
	auto tok = tok_res.unwrap();

	return ast.ValueId(span, ast.create_string(tok.start, tok.size));
}

AstNode*
ast::to_type_identifier(Ast& ast, ConsumeResult const& tok_res, Span span)
{
	auto tok = tok_res.unwrap();

	return ast.TypeId(span, ast.create_string(tok.start, tok.size));
}
