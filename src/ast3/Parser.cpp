

#include "Parser.h"

template<typename T>
static ParseError
MoveError(ParseResult<T>& err)
{
	return ParseError(*err.unwrap_error().get());
}

static ParseError
NotImpl()
{
	return ParseError("Not Implemented.");
}

static std::string
to_string(Token token)
{
	return std::string(token.view.start, token.view.size);
}

ParseResult<std::vector<std::string>>
Parser::parse_name_parts()
{
	std::vector<std::string> name_parts;
	auto tok = cursor.consume(TokenKind::Identifier);
	if( !tok.ok() )
		return ParseError("Expected identifier", tok.token());

	Token tok_val = tok.token();
	name_parts.emplace_back(to_string(tok_val));

	while( cursor.consume_if_expected(TokenKind::ColonColon).ok() )
	{
		tok = cursor.consume(TokenKind::Identifier);
		if( !tok.ok() )
			return ParseError("Expected identifier", tok.token());

		tok_val = tok.token();
		name_parts.emplace_back(to_string(tok_val));
	}

	return name_parts;
}

ParseResult<std::vector<AstNode*>>
Parser::parse_func_params()
{
	// auto trail = get_parse_trail();
	std::vector<AstNode*> params;

	Token curr_tok = cursor.peek();
	while( curr_tok.kind != TokenKind::CloseParen )
	{
		auto decl = parse_var_decl(false);
		if( !decl.ok() )
			return MoveError(decl);

		// Also catches trailing comma.
		cursor.consume_if_expected(TokenKind::Comma);

		params.push_back(decl.unwrap());
		curr_tok = cursor.peek();
	}

	return params;
}

ParseResult<AstNode*>
Parser::parse(Ast& ast, Cursor& cursor)
{
	Parser parser(ast, cursor);
	return parser.parse_module();
}

ParseResult<AstNode*>
Parser::parse_module()
{
	std::vector<AstNode*> statements;

	while( !cursor.at_end() )
	{
		ParseResult<AstNode*> statement = parse_module_statement();
		if( !statement.ok() )
			return statement;

		statements.push_back(statement.unwrap());
	}

	return ast.create<AstModule>(Span(), statements);
}

ParseResult<AstNode*>
Parser::parse_module_statement()
{
	Token tok = cursor.peek();
	switch( tok.kind )
	{
	case TokenKind::FnKw:
		return parse_function();
	default:
		return ParseError("Expected top level 'fn' or 'struct' declaration.");
	}

	return ParseError("??.");
}

ParseResult<AstNode*>
Parser::parse_identifier()
{
	// auto trail = get_parse_trail();

	auto name_parts = parse_name_parts();
	if( !name_parts.ok() )
		return ParseError(*name_parts.unwrap_error().get());

	return ast.create<AstId>(Span(), name_parts.unwrap());
}

ParseResult<AstNode*>
Parser::parse_type_decl(bool allow_empty)
{
	// auto trail = get_parse_trail();

	auto tok = cursor.peek();
	if( tok.kind != TokenKind::Identifier )
		return ParseError("Unexpected token while parsing type.", tok);

	auto id_result = parse_identifier();
	if( !id_result.ok() )
		return id_result;

	return ast.create<AstTypeDeclarator>(Span(), id_result.unwrap());
}

ParseResult<AstNode*>
Parser::parse_var_decl(bool allow_untyped)
{
	auto id = parse_identifier();
	if( !id.ok() )
		return id;

	auto type_tok = cursor.consume_if_expected(TokenKind::Colon);
	if( !type_tok.ok() && !allow_untyped )
		return ParseError("Expected type.", type_tok.token());

	if( !type_tok.ok() )
		return ast.create<AstVarDecl>(Span(), id.unwrap(), nullptr);
	else
	{
		auto type_decl = parse_type_decl(false);
		if( !type_decl.ok() )
			return type_decl;

		return ast.create<AstVarDecl>(Span(), id.unwrap(), type_decl.unwrap());
	}
}

ParseResult<AstNode*>
Parser::parse_function()
{
	// auto trail = get_parse_trail();

	auto tok = cursor.consume(TokenKind::FnKw);
	if( !tok.ok() )
		return ParseError("Expected 'fn'", tok.token());

	auto proto = parse_function_proto();
	if( !proto.ok() )
		return proto;

	auto definition = parse_function_body();
	if( !definition.ok() )
		return definition;

	return ast.create<AstFunc>(Span(), proto.unwrap(), definition.unwrap());
}

ParseResult<AstNode*>
Parser::parse_function_proto()
{
	// auto trail = get_parse_trail();

	auto id_result = parse_identifier();
	if( !id_result.ok() )
		return id_result;

	auto tok = cursor.consume(TokenKind::OpenParen);
	if( !tok.ok() )
		return ParseError("Expected '('", tok.token());

	auto parameters = parse_func_params();
	if( !parameters.ok() )
		return ParseError("Failed parsing func params");

	tok = cursor.consume(TokenKind::CloseParen);
	if( !tok.ok() )
		return ParseError("Expected ')'", tok.token());

	tok = cursor.consume(TokenKind::Colon);
	if( !tok.ok() )
		return ParseError("Expected ':'", tok.token());

	auto return_type_decl = parse_type_decl(false);
	if( !return_type_decl.ok() )
		return return_type_decl;

	// Linkage linkage, AstNode* id, std::vector<AstNode*> parameters, AstNode* rt_type_declarator
	return ast.create<AstFuncProto>(
		Span(),
		AstFuncProto::Linkage::Extern,
		id_result.unwrap(),
		parameters.unwrap(),
		return_type_decl.unwrap());

	// if( infer_type_tok.ok() )
	// {
	// 	auto return_type_identifier = parse_type_decl(false);
	// 	if( !return_type_identifier.ok() )
	// 	{
	// 		return return_type_identifier;
	// 	}
	// 	return ast.FnProto(
	// 		Span(), fn_identifier.unwrap(), params.unwrap(), return_type_identifier.unwrap());
	// }
	// else
	// {
	// 	return ast.FnProto(
	// 		Span(), fn_identifier.unwrap(), ast.TypeDeclaratorEmpty(), params.unwrap());
	// }
}

ParseResult<AstNode*>
Parser::parse_function_body()
{
	return parse_block();
}

ParseResult<AstNode*>
Parser::parse_block()
{
	// auto trail = get_parse_trail();

	std::vector<AstNode*> statements;

	auto tok = cursor.consume(TokenKind::OpenCurly);
	if( !tok.ok() )
		return ParseError("Expected '{'", tok.token());

	auto curr_tok = cursor.peek();
	while( curr_tok.kind != TokenKind::CloseCurly )
	{
		auto stmt = parse_statement();
		if( !stmt.ok() )
			return stmt;

		statements.push_back(stmt.unwrap());

		curr_tok = cursor.peek();
	}

	tok = cursor.consume(TokenKind::CloseCurly);
	if( !tok.ok() )
		return ParseError("Expected '}'", tok.token());

	return ast.create<AstBlock>(Span(), statements);
}

ParseResult<AstNode*>
Parser::parse_number_literal()
{
	auto tok = cursor.consume(TokenKind::NumberLiteral);
	if( !tok.ok() )
		return ParseError("Expected number literal", tok.token());

	std::string sz = to_string(tok.token());
	int val = std::stoi(sz);
	return ast.create<AstNumberLiteral>(Span(), val);
}

ParseResult<AstNode*>
Parser::parse_postfix_expr()
{
	return parse_simple_expr();
	// 	auto trail = get_parse_trail();

	// 	auto expr = parse_simple_expr();
	// 	if( !expr.ok() )
	// 	{
	// 		return expr;
	// 	}

	// 	while( true )
	// 	{
	// 		auto tok = cursor.peek();

	// 		switch( tok.type )
	// 		{
	// 		case TokenType::dot:
	// 			// Member dereference
	// 			expr = parse_member_reference(expr.unwrap());
	// 			if( !expr.ok() )
	// 			{
	// 				return expr;
	// 			}
	// 			expr = ast.Expr(trail.mark(), expr.unwrap());
	// 			break;
	// 		case TokenType::indirect_member_access:
	// 			// Member dereference
	// 			expr = parse_indirect_member_reference(expr.unwrap());
	// 			if( !expr.ok() )
	// 			{
	// 				return expr;
	// 			}
	// 			expr = ast.Expr(trail.mark(), expr.unwrap());
	// 			break;
	// 		case TokenType::open_paren:
	// 			expr = parse_call(expr.unwrap());
	// 			if( !expr.ok() )
	// 			{
	// 				return expr;
	// 			}
	// 			expr = ast.Expr(trail.mark(), expr.unwrap());
	// 			break;
	// 		case TokenType::open_square:
	// 			expr = parse_array_access(expr.unwrap());
	// 			if( !expr.ok() )
	// 			{
	// 				return expr;
	// 			}
	// 			expr = ast.Expr(trail.mark(), expr.unwrap());
	// 			break;
	// 		default:
	// 			goto done;
	// 			break;
	// 		}
	// 	}
	// done:
	// 	return expr;
}

ParseResult<AstNode*>
Parser::parse_simple_expr()
{
	// auto trail = get_parse_trail();
	AstNode* result = nullptr;
	auto tok = cursor.peek();
	switch( tok.kind )
	{
	case TokenKind::NumberLiteral:
	{
		auto expr = parse_number_literal();
		if( !expr.ok() )
			return expr;
		result = expr.unwrap();
		break;
	}
	default:
		return NotImpl();
	}

	return ast.create<AstExpr>(Span(), result);
}

ParseResult<AstNode*>
Parser::parse_bin_op(int expr_precidence, AstNode* lhs)
{
	return NotImpl();
	// auto trail = get_parse_trail();
	// // If this is a binop, find its precedence.
	// while( true )
	// {
	// 	// This is a binary operation because TokPrec would be less than ExprPrec if
	// 	// the next token was not a bin op (e.g. if statement or so.)

	// 	// TODO: Consume bin op
	// 	auto tok = cursor.consume(
	// 		{TokenKind::plus,
	// 		 TokenKind::star,
	// 		 TokenKind::slash,
	// 		 TokenKind::minus,
	// 		 TokenKind::gt,
	// 		 TokenKind::gte,
	// 		 TokenKind::lt,
	// 		 TokenKind::lte,
	// 		 TokenKind::and_and_lex,
	// 		 TokenKind::or_or_lex,
	// 		 TokenKind::cmp,
	// 		 TokenKind::ne}); // TODO: Other assignment exprs.
	// 	if( !tok.ok() )
	// 		return lhs;

	// 	auto token_type = tok.as().type;
	// 	auto op = get_bin_op_from_token_type(token_type);
	// 	int tok_precidence = get_token_precedence(op);

	// 	// If this is a binop that binds at least as tightly as the current binop,
	// 	// consume it, otherwise we are done.
	// 	if( tok_precidence < expr_precidence )
	// 		return lhs;

	// 	// Parse the primary expression after the binary operator.
	// 	auto rhs = parse_postfix_expr();
	// 	if( !rhs.ok() )
	// 		return rhs;

	// 	// If BinOp binds less tightly with RHS than the operator after RHS, let
	// 	// the pending operator take RHS as its LHS.
	// 	auto cur = cursor.peek();
	// 	token_type = cur.type;
	// 	auto next_op = get_bin_op_from_token_type(token_type);
	// 	int next_precidence = get_token_precedence(next_op);
	// 	if( tok_precidence < next_precidence )
	// 	{
	// 		rhs = parse_bin_op(tok_precidence + 1, rhs.unwrap());
	// 		if( !rhs.ok() )
	// 			return rhs;
	// 	}

	// 	// Merge LHS/RHS.
	// 	lhs = ast.Expr(trail.mark(), ast.BinOp(trail.mark(), op, lhs, rhs.unwrap()));
	// }
}

ParseResult<AstNode*>
Parser::parse_expr()
{
	// auto trail = get_parse_trail();
	auto lhs = parse_postfix_expr();
	if( !lhs.ok() )
		return lhs;

	return lhs;
	// auto op = parse_bin_op(0, lhs.unwrap());
	// if( !op.ok() )
	// 	return op;

	// if( cursor.peek().type == TokenType::is )
	// {
	// 	cursor.consume(TokenType::is);

	// 	auto type_id = parse_type_decl(false);
	// 	if( !type_id.ok() )
	// 		return type_id;

	// 	op = ast.Is(trail.mark(), op.unwrap(), type_id.unwrap());
	// }
	// else if( cursor.peek().type == TokenType::open_curly )
	// {
	// 	auto initializer_block = parse_initializer(op.unwrap());
	// 	if( !initializer_block.ok() )
	// 		return initializer_block;

	// 	op = initializer_block.unwrap();
	// }

	// return op;
}

ParseResult<AstNode*>
Parser::parse_statement()
{
	// auto trail = get_parse_trail();

	auto tok = cursor.peek();

	AstNode* stmt = nullptr;
	switch( tok.kind )
	{
	case TokenKind::ReturnKw:
	{
		cursor.consume_if_expected(TokenKind::ReturnKw);

		auto return_expr = parse_expr();
		if( !return_expr.ok() )
			return return_expr;

		stmt = ast.create<AstReturn>(Span(), return_expr.unwrap());
		break;
	}
	default:
		return NotImpl();
	}

	{
		auto curr_tok = cursor.consume(TokenKind::SemiColon);
		if( !curr_tok.ok() )
			return ParseError("Expected ';'", curr_tok.token());
	}

no_semi:
	return ast.create<AstStmt>(Span(), stmt);
}