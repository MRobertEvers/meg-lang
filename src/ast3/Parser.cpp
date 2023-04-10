

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

ParseResult<std::vector<AstNode*>>
Parser::parse_struct_body()
{
	std::vector<AstNode*> members;

	auto tok = cursor.consume(TokenKind::OpenCurly);
	if( !tok.ok() )
		return ParseError("Expected '{'", tok.token());

	auto curr_tok = cursor.peek();
	while( curr_tok.kind != TokenKind::CloseCurly )
	{
		auto var_decl_result = parse_var_decl(false);
		if( !var_decl_result.ok() )
			return MoveError(var_decl_result);

		members.push_back(var_decl_result.unwrap());

		// Optional semicolon
		cursor.consume_if_expected(TokenKind::SemiColon);

		curr_tok = cursor.peek();
	}

	tok = cursor.consume(TokenKind::CloseCurly);
	if( !tok.ok() )
		return ParseError("Expected '}'", tok.token());

	return members;
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
	case TokenKind::StructKw:
		return parse_struct();
	case TokenKind::UnionKw:
		return parse_union();
	case TokenKind::EnumKw:
		return parse_enum();
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
Parser::parse_call(AstNode* callee)
{
	// auto trail = get_parse_trail();
	std::vector<AstNode*> args;

	ConsumeResult tok = cursor.consume(TokenKind::OpenParen);
	if( !tok.ok() )
		return ParseError("Expected '('", tok.token());

	Token curr_tok = cursor.peek();
	while( curr_tok.kind != TokenKind::CloseParen )
	{
		auto expr_result = parse_expr();
		if( !expr_result.ok() )
			return expr_result;

		args.push_back(expr_result.unwrap());

		// Also catches trailing comma.
		cursor.consume_if_expected(TokenKind::Comma);
		curr_tok = cursor.peek();
	}

	tok = cursor.consume(TokenKind::CloseParen);
	if( !tok.ok() )
		return ParseError("Expected ')'", tok.token());

	return ast.create<AstFuncCall>(Span(), callee, args);
}

ParseResult<AstNode*>
Parser::parse_indirect_member_access(AstNode* base)
{
	ConsumeResult tok = cursor.consume(TokenKind::SkinnyArrow);
	if( !tok.ok() )
		return ParseError("Expected '->'", tok.token());

	auto expr_result = parse_identifier();
	if( !expr_result.ok() )
		return expr_result;

	return ast.create<AstMemberAccess>(
		Span(), base, expr_result.unwrap(), AstMemberAccess::AccessKind::Indirect);
}

ParseResult<AstNode*>
Parser::parse_member_access(AstNode* base)
{
	ConsumeResult tok = cursor.consume(TokenKind::Dot);
	if( !tok.ok() )
		return ParseError("Expected '.'", tok.token());

	auto expr_result = parse_identifier();
	if( !expr_result.ok() )
		return expr_result;

	return ast.create<AstMemberAccess>(
		Span(), base, expr_result.unwrap(), AstMemberAccess::AccessKind::Direct);
}

ParseResult<AstNode*>
Parser::parse_array_access(AstNode* array)
{
	// auto trail = get_parse_trail();

	ConsumeResult tok = cursor.consume(TokenKind::OpenSquare);
	if( !tok.ok() )
		return ParseError("Expected '['", tok.token());

	auto expr_result = parse_expr();
	if( !expr_result.ok() )
		return expr_result;

	tok = cursor.consume(TokenKind::CloseSquare);
	if( !tok.ok() )
		return ParseError("Expected ']'", tok.token());

	return ast.create<AstArrayAccess>(Span(), array, expr_result.unwrap());
}

ParseResult<AstNode*>
Parser::parse_deref()
{
	ConsumeResult tok = cursor.consume(TokenKind::Star);
	if( !tok.ok() )
		return ParseError("Expected '*'", tok.token());

	auto expr_result = parse_postfix_expr();
	if( !expr_result.ok() )
		return expr_result;

	return ast.create<AstDeref>(Span(), expr_result.unwrap());
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
Parser::parse_struct()
{
	std::vector<AstNode*> members;

	auto tok = cursor.consume(TokenKind::StructKw);
	if( !tok.ok() )
		return ParseError("Expected 'struct'", tok.token());

	auto id_result = parse_identifier();
	if( !id_result.ok() )
		return id_result;

	auto body_result = parse_struct_body();
	if( !body_result.ok() )
		return MoveError(body_result);

	return ast.create<AstStruct>(Span(), id_result.unwrap(), body_result.unwrap());
}

ParseResult<AstNode*>
Parser::parse_union()
{
	auto tok = cursor.consume(TokenKind::UnionKw);
	if( !tok.ok() )
		return ParseError("Expected 'union'", tok.token());

	auto id_result = parse_identifier();
	if( !id_result.ok() )
		return id_result;

	auto body_result = parse_struct_body();
	if( !body_result.ok() )
		return MoveError(body_result);

	return ast.create<AstUnion>(Span(), id_result.unwrap(), body_result.unwrap());
}

ParseResult<AstNode*>
Parser::parse_enum()
{
	std::vector<AstNode*> members;

	auto tok = cursor.consume(TokenKind::EnumKw);
	if( !tok.ok() )
		return ParseError("Expected 'enum'", tok.token());

	auto id_result = parse_identifier();
	if( !id_result.ok() )
		return id_result;

	tok = cursor.consume(TokenKind::OpenCurly);
	if( !tok.ok() )
		return ParseError("Expected '{'", tok.token());

	auto curr_tok = cursor.peek();
	while( curr_tok.kind != TokenKind::CloseCurly )
	{
		auto enum_member_result = parse_enum_member();
		if( !enum_member_result.ok() )
			return MoveError(enum_member_result);

		members.push_back(enum_member_result.unwrap());

		// Optional comma
		cursor.consume_if_expected(TokenKind::Comma);

		curr_tok = cursor.peek();
	}

	tok = cursor.consume(TokenKind::CloseCurly);
	if( !tok.ok() )
		return ParseError("Expected '}'", tok.token());

	return ast.create<AstEnum>(Span(), id_result.unwrap(), members);
}

ParseResult<AstNode*>
Parser::parse_enum_member()
{
	auto id_result = parse_identifier();
	if( !id_result.ok() )
		return id_result;

	auto tok = cursor.peek();
	if( tok.kind == TokenKind::OpenCurly )
	{
		auto body_result = parse_struct_body();
		if( !body_result.ok() )
			return MoveError(body_result);

		return ast.create<AstEnumMember>(Span(), id_result.unwrap(), body_result.unwrap());
	}
	else
		return ast.create<AstEnumMember>(Span(), id_result.unwrap());
}

ParseResult<AstNode*>
Parser::parse_let()
{
	// auto trail = get_parse_trail();

	std::vector<AstNode*> statements;

	auto tok = cursor.consume(TokenKind::LetKw);
	if( !tok.ok() )
		return ParseError("Expected 'let'", tok.token());

	auto ast_result = parse_var_decl(true);
	if( !ast_result.ok() )
		return ast_result;

	tok = cursor.consume_if_expected(TokenKind::Eq);
	if( !tok.ok() )
		return ast.create<AstLet>(Span(), ast_result.unwrap(), nullptr);

	auto rhs_result = parse_expr();
	if( !rhs_result.ok() )
		return rhs_result;

	return ast.create<AstLet>(Span(), ast_result.unwrap(), rhs_result.unwrap());
}

ParseResult<AstNode*>
Parser::parse_if()
{
	auto tok = cursor.consume(TokenKind::IfKw);
	if( !tok.ok() )
		return ParseError("Expected 'if'", tok.token());

	auto expr_result = parse_expr();
	if( !expr_result.ok() )
		return expr_result;

	auto then_result = parse_statement();
	if( !then_result.ok() )
		return then_result;

	tok = cursor.consume_if_expected(TokenKind::ElseKw);
	if( !tok.ok() )
		return ast.create<AstIf>(Span(), expr_result.unwrap(), then_result.unwrap(), nullptr);

	auto else_result = parse_statement();
	if( !else_result.ok() )
		return else_result;

	return ast.create<AstIf>(
		Span(), expr_result.unwrap(), then_result.unwrap(), else_result.unwrap());
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
	auto simple_expr_result = parse_simple_expr();
	if( !simple_expr_result.ok() )
		return simple_expr_result;

	Token tok = cursor.peek();

	switch( tok.kind )
	{
	case TokenKind::OpenSquare:
		return parse_array_access(simple_expr_result.unwrap());
	case TokenKind::OpenParen:
		return parse_call(simple_expr_result.unwrap());
	case TokenKind::SkinnyArrow:
		return parse_indirect_member_access(simple_expr_result.unwrap());
	case TokenKind::Dot:
		return parse_member_access(simple_expr_result.unwrap());
	default:
		return simple_expr_result.unwrap();
	}
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
	Token tok = cursor.peek();
	switch( tok.kind )
	{
	case TokenKind::NumberLiteral:
	{
		auto expr_result = parse_number_literal();
		if( !expr_result.ok() )
			return expr_result;
		result = expr_result.unwrap();
		break;
	}
	case TokenKind::Identifier:
	{
		cursor.consume(TokenKind::Identifier);
		result = ast.create<AstId>(Span(), to_string(tok));
		break;
	}
	case TokenKind::SizeOfKw:
	{
		cursor.consume(TokenKind::SizeOfKw);

		auto expr_result = parse_expr();
		if( !expr_result.ok() )
			return expr_result;

		result = ast.create<AstSizeOf>(Span(), expr_result.unwrap());
		break;
	}
	case TokenKind::OpenParen:
	{
		// Eat ALL unnecessary parens.
		int open_count = 0;
		do
		{
			cursor.consume(TokenKind::OpenParen);
			open_count += 1;
		} while( cursor.peek().kind == TokenKind::OpenParen );

		auto expr_result = parse_expr_any();
		if( !expr_result.ok() )
			return expr_result;

		for( int i = 0; i < open_count; i++ )
		{
			auto cons = cursor.consume(TokenKind::CloseParen);
			if( !cons.ok() )
				return ParseError("Expected ')'", cons.token());
		}

		result = expr_result.unwrap();
		break;
	}
	case TokenKind::Ampersand:
	{
		cursor.consume(TokenKind::Ampersand);
		auto expr_result = parse_postfix_expr();
		if( !expr_result.ok() )
			return expr_result;

		result = ast.create<AstAddressOf>(Span(), expr_result.unwrap());
		break;
	}
	case TokenKind::Exclam:
	{
		cursor.consume(TokenKind::Exclam);
		auto expr_result = parse_postfix_expr();
		if( !expr_result.ok() )
			return expr_result;

		result = ast.create<AstBoolNot>(Span(), expr_result.unwrap());
		break;
	}
	case TokenKind::Star:
	{
		auto expr_result = parse_deref();
		if( !expr_result.ok() )
			return expr_result;

		result = expr_result.unwrap();
		break;
	}
	default:
		return NotImpl();
	}

	return result;
}

ParseResult<AstNode*>
Parser::parse_bin_op(int expr_precidence, AstNode* lhs)
{
	// auto trail = get_parse_trail();
	// If this is a binop, find its precedence.
	while( true )
	{
		// This is a binary operation because TokPrec would be less than ExprPrec if
		// the next token was not a bin op (e.g. if statement or so.)

		ConsumeResult consume = cursor.consume({
			TokenKind::Star,
			TokenKind::Slash,
			TokenKind::Plus,
			TokenKind::Minus,
			TokenKind::EqEq,
			TokenKind::Gt,
			TokenKind::GtEq,
			TokenKind::Lt,
			TokenKind::LtEq,
			TokenKind::ExclamEq,
		});
		// TODO: Other assignment exprs.
		if( !consume.ok() )
			return lhs;

		Token tok = consume.token();

		TokenKind token_type = tok.kind;
		BinOp op = get_bin_op_from_token_type(token_type);
		int tok_precidence = get_token_precedence(op);

		// If this is a binop that binds at least as tightly as the current binop,
		// consume it, otherwise we are done.
		if( tok_precidence < expr_precidence )
			return lhs;

		// Parse the primary expression after the binary operator.
		auto rhs_result = parse_postfix_expr();
		if( !rhs_result.ok() )
			return rhs_result;

		// If BinOp binds less tightly with RHS than the operator after RHS, let
		// the pending operator take RHS as its LHS.
		Token next_tok = cursor.peek();
		token_type = next_tok.kind;
		BinOp next_op = get_bin_op_from_token_type(token_type);
		int next_precidence = get_token_precedence(next_op);
		if( tok_precidence < next_precidence )
		{
			rhs_result = parse_bin_op(tok_precidence + 1, rhs_result.unwrap());
			if( !rhs_result.ok() )
				return rhs_result;
		}

		// Merge LHS/RHS.
		lhs = ast.create<AstBinOp>(Span(), op, lhs, rhs_result.unwrap());
	}
}

ParseResult<AstNode*>
Parser::parse_expr_any()
{
	// auto trail = get_parse_trail();
	auto lhs_result = parse_postfix_expr();
	if( !lhs_result.ok() )
		return lhs_result;

	auto bin_op_result = parse_bin_op(0, lhs_result.unwrap());
	if( !bin_op_result.ok() )
		return bin_op_result;

	return bin_op_result;
}

ParseResult<AstNode*>
Parser::parse_expr()
{
	// auto trail = get_parse_trail();
	auto expr_any_result = parse_expr_any();
	if( !expr_any_result.ok() )
		return expr_any_result;

	return ast.create<AstExpr>(Span(), expr_any_result.unwrap());

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
		cursor.consume(TokenKind::ReturnKw);

		auto return_expr = parse_expr();
		if( !return_expr.ok() )
			return return_expr;

		stmt = ast.create<AstReturn>(Span(), return_expr.unwrap());
		break;
	}
	case TokenKind::LetKw:
	{
		auto let_stmt = parse_let();
		if( !let_stmt.ok() )
			return let_stmt;

		stmt = let_stmt.unwrap();
		break;
	}
	case TokenKind::IfKw:
	{
		auto if_stmt = parse_if();
		if( !if_stmt.ok() )
			return if_stmt;

		stmt = if_stmt.unwrap();
		goto no_semi;
		break;
	}
	case TokenKind::OpenCurly:
	{
		auto block_stmt = parse_block();
		if( !block_stmt.ok() )
			return block_stmt;

		stmt = block_stmt.unwrap();
		goto no_semi;
		break;
	}
	default:
	{
		auto expr_stmt = parse_expr();
		if( !expr_stmt.ok() )
			return expr_stmt;

		stmt = expr_stmt.unwrap();
		break;
	}
	}

	{
		auto curr_tok = cursor.consume(TokenKind::SemiColon);
		if( !curr_tok.ok() )
			return ParseError("Expected ';'", curr_tok.token());
	}

no_semi:
	return ast.create<AstStmt>(Span(), stmt);
}