#include "AstGen.h"

#include "Ast.h"
#include "bin_op.h"

#include <string>

using namespace ast;

static AstNode*
to_value_identifier(Ast& ast, ConsumeResult const& tok_res, Span span)
{
	auto tok = tok_res.unwrap();

	return ast.ValueId(span, ast.create_string(tok.start, tok.size));
}

static AstNode*
to_type_identifier(Ast& ast, ConsumeResult const& tok_res, Span span)
{
	auto tok = tok_res.unwrap();

	return ast.TypeId(span, ast.create_string(tok.start, tok.size));
}

AstGen::AstGen(Ast& ast, TokenCursor& cursor)
	: ast(ast)
	, cursor(cursor)
{
	init_bin_op_lookup();
}

ParseResult<ast::AstNode*>
AstGen::parse()
{
	auto trail = get_parse_trail();

	auto nodes = ast.create_list();

	while( cursor.has_tokens() && cursor.peek().type != TokenType::eof )
	{
		auto item = parse_module_top_level_item();
		if( !item.ok() )
		{
			return item;
		}
		nodes->append(item.unwrap());
	}

	return ast.Module(trail.mark(), nodes);
}

ParseResult<ast::AstNode*>
AstGen::parse_module_top_level_item()
{
	auto tok = cursor.peek();
	switch( tok.type )
	{
	// Fall through
	case TokenType::extern_keyword:
		return parse_extern_function();
	case TokenType::fn:
		return parse_function();
	case TokenType::struct_keyword:
		return parse_struct();
	default:
		return ParseError("Expected top level 'fn' or 'struct' declaration.");
	}
}

ParseResult<ast::AstNode*>
AstGen::parse_let()
{
	auto trail = get_parse_trail();

	auto tok = cursor.consume(TokenType::let);
	if( !tok.ok() )
	{
		return ParseError("Expected 'let'", tok.as());
	}

	auto identifier = parse_identifier();
	if( !identifier.ok() )
	{
		return identifier;
	}

	tok = cursor.consume(TokenType::colon, TokenType::equal);
	if( !tok.ok() )
	{
		return ParseError("Expected identifier or '='", tok.as());
	}

	auto type_decl = ast.TypeDeclaratorEmpty();
	if( tok.unwrap().type == TokenType::colon )
	{
		auto type_decl_result = parse_type_decl(true);
		if( !type_decl_result.ok() )
		{
			return type_decl_result;
		}

		type_decl = type_decl_result.unwrap();
	}

	cursor.consume_if_expected(TokenType::equal);

	auto expr = parse_expr();
	if( !expr.ok() )
	{
		return expr;
	}

	return ast.Let(trail.mark(), identifier.unwrap(), type_decl, expr.unwrap());
}

ParseResult<ast::AstNode*>
AstGen::parse_block()
{
	auto trail = get_parse_trail();

	auto stmts = ast.create_list();

	auto tok = cursor.consume(TokenType::open_curly);
	if( !tok.ok() )
	{
		return ParseError("Expected '{'", tok.as());
	}

	auto curr_tok = cursor.peek();
	while( curr_tok.type != TokenType::close_curly )
	{
		auto stmt = parse_statement();
		if( !stmt.ok() )
		{
			return stmt;
		}

		stmts->append(stmt.unwrap());

		curr_tok = cursor.peek();
	}
	cursor.consume_if_expected(TokenType::close_curly);

	return ast.Block(trail.mark(), stmts);
}

ParseResult<ast::AstNode*>
AstGen::parse_struct()
{
	auto trail = get_parse_trail();

	auto members = ast.create_list();

	auto consume_tok = cursor.consume(TokenType::struct_keyword);
	if( !consume_tok.ok() )
	{
		return ParseError("Expected 'struct'", consume_tok.as());
	}

	consume_tok = cursor.consume(TokenType::identifier);
	if( !consume_tok.ok() )
	{
		return ParseError("Expected struct identifier.", consume_tok.as());
	}

	auto struct_name = to_type_identifier(ast, consume_tok, trail.mark());

	consume_tok = cursor.consume(TokenType::open_curly);
	if( !consume_tok.ok() )
	{
		return ParseError("Expected block, '{'.", consume_tok.as());
	}

	auto tok = cursor.peek();
	while( tok.type != TokenType::close_curly )
	{
		auto member_trail = get_parse_trail();

		consume_tok = cursor.consume(TokenType::identifier);
		if( !consume_tok.ok() )
		{
			return ParseError("Expected member declaration.", consume_tok.as());
		}

		auto name = to_value_identifier(ast, consume_tok, member_trail.mark());

		consume_tok = cursor.consume(TokenType::colon);
		if( !consume_tok.ok() )
		{
			return ParseError("Expected ':'.", consume_tok.as());
		}

		auto decl = parse_type_decl(false);
		if( !decl.ok() )
		{
			return decl;
		}

		consume_tok = cursor.consume(TokenType::semicolon);
		if( !consume_tok.ok() )
		{
			return ParseError("Expected ';'.", consume_tok.as());
		}

		members->append(ast.ValueDecl(member_trail.mark(), name, decl.unwrap()));

		tok = cursor.peek();
	}

	consume_tok = cursor.consume(TokenType::close_curly);

	return ast.Struct(trail.mark(), struct_name, members);
}

ParseResult<ast::AstNode*>
AstGen::parse_type_decl(bool allow_empty)
{
	auto trail = get_parse_trail();

	auto tok = cursor.consume_if_expected(TokenType::identifier);
	if( !tok.ok() )
	{
		if( allow_empty )
		{
			return ast.TypeDeclaratorEmpty();
		}
		else
		{
			return ParseError("Unexpected token while parsing type.", tok.as());
		}
	}

	auto tok_val = tok.as();
	auto name = ast.create_string(tok_val.start, tok_val.size);

	int indirection_count = 0;
	auto star_tok = cursor.consume_if_expected(TokenType::star);
	while( star_tok.ok() )
	{
		indirection_count += 1;
		star_tok = cursor.consume_if_expected(TokenType::star);
	}

	return ast.TypeDeclarator(trail.mark(), name, indirection_count);
}

ParseResult<ast::AstNode*>
AstGen::parse_bin_op(int expr_precidence, ast::AstNode* lhs)
{
	auto trail = get_parse_trail();
	// If this is a binop, find its precedence.
	while( true )
	{
		auto cur = cursor.peek();

		// This is a binary operation because TokPrec would be less than ExprPrec if
		// the next token was not a bin op (e.g. if statement or so.)

		// TODO: Consume bin op
		auto tok = cursor.consume(
			{TokenType::plus,
			 TokenType::star,
			 TokenType::slash,
			 TokenType::minus,
			 TokenType::gt,
			 TokenType::gte,
			 TokenType::lt,
			 TokenType::lte,
			 TokenType::and_lex,
			 TokenType::or_lex,
			 TokenType::cmp,
			 TokenType::ne});

		auto token_type = tok.as().type;
		auto op = get_bin_op_from_token_type(token_type);
		int tok_precidence = get_token_precedence(op);

		// If this is a binop that binds at least as tightly as the current binop,
		// consume it, otherwise we are done.
		if( tok_precidence < expr_precidence )
			return lhs;

		// Parse the primary expression after the binary operator.
		auto rhs = parse_postfix_expr();
		if( !rhs.ok() )
			return rhs;

		// If BinOp binds less tightly with RHS than the operator after RHS, let
		// the pending operator take RHS as its LHS.
		cur = cursor.peek();
		token_type = tok.as().type;
		op = get_bin_op_from_token_type(token_type);
		int next_precidence = get_token_precedence(op);
		if( tok_precidence < next_precidence )
		{
			rhs = parse_bin_op(tok_precidence + 1, rhs.unwrap());
			if( !rhs.ok() )
				return rhs;
		}

		// Merge LHS/RHS.
		lhs = ast.Expr(trail.mark(), ast.BinOp(trail.mark(), op, lhs, rhs.unwrap()));
	}
}

ParseResult<ast::AstNode*>
AstGen::parse_assign(ast::AstNode* lhs)
{
	auto trail = get_parse_trail();
	auto tok = cursor.consume({
		TokenType::plus_equal,
		TokenType::sub_equal,
		TokenType::mul_equal,
		TokenType::div_equal,
		TokenType::equal,
	});
	if( !tok.ok() )
	{
		return ParseError("Expected '='", tok.as());
	}

	auto rhs = parse_expr();
	if( !rhs.ok() )
	{
		return rhs;
	}

	AssignOp op = AssignOp::assign;
	switch( tok.as().type )
	{
	case TokenType::equal:
		op = AssignOp::assign;
		break;
	case TokenType::plus_equal:
		op = AssignOp::add;
		break;
	case TokenType::sub_equal:
		op = AssignOp::sub;
		break;
	case TokenType::mul_equal:
		op = AssignOp::mul;
		break;
	case TokenType::div_equal:
		op = AssignOp::div;
		break;

	default:
		return ParseError("Unexpected token.", tok.as());
		break;
	}

	return ast.Assign(trail.mark(), op, lhs, rhs.unwrap());
}

ParseResult<ast::AstNode*>
AstGen::parse_expr_statement()
{
	auto expr = parse_expr();
	if( !expr.ok() )
	{
		return expr;
	}

	auto tok = cursor.peek();
	switch( tok.type )
	{
	case TokenType::equal:
	case TokenType::mul_equal:
	case TokenType::div_equal:
	case TokenType::sub_equal:
	case TokenType::plus_equal:
		return parse_assign(expr.unwrap());
		break;

	default:
		return expr;
		break;
	}
}

ParseResult<ast::AstNode*>
AstGen::parse_while()
{
	auto trail = get_parse_trail();

	auto tok = cursor.consume(TokenType::while_keyword);
	if( !tok.ok() )
	{
		return ParseError("Expected while keyword", tok.as());
	}

	auto open_paren_tok = cursor.consume_if_expected(TokenType::open_paren);

	auto condition = parse_expr();
	if( !condition.ok() )
	{
		return condition;
	}

	if( open_paren_tok.ok() )
	{
		tok = cursor.consume(TokenType::close_paren);
		if( !tok.ok() )
		{
			return ParseError("Expected ')'", tok.as());
		}
	}

	auto loop_block = parse_statement();
	if( !loop_block.ok() )
	{
		return loop_block;
	}

	return ast.While(trail.mark(), condition.unwrap(), loop_block.unwrap());
}

ParseResult<ast::AstNode*>
AstGen::parse_statement()
{
	auto trail = get_parse_trail();

	auto tok = cursor.peek();

	AstNode* stmt = nullptr;
	switch( tok.type )
	{
	case TokenType::return_keyword:
	{
		// TODO: Parse return.
		cursor.consume_if_expected(TokenType::return_keyword);

		auto return_expr = parse_expr();
		if( !return_expr.ok() )
		{
			return return_expr;
		}

		stmt = ast.Return(trail.mark(), return_expr.unwrap());
		break;
	}
	case TokenType::while_keyword:
	{
		auto expr = parse_while();
		if( !expr.ok() )
		{
			return expr;
		}

		stmt = expr.unwrap();

		goto no_semi;
	}
	break;
	case TokenType::let:
	{
		auto expr = parse_let();
		if( !expr.ok() )
		{
			return expr;
		}

		stmt = expr.unwrap();
	}
	break;
	case TokenType::if_keyword:
	{
		auto expr = parse_if();
		if( !expr.ok() )
		{
			return expr;
		}

		stmt = expr.unwrap();

		// TODO: Better way to do this?
		goto no_semi;
	}
	break;
	case TokenType::for_keyword:
	{
		auto expr = parse_for();
		if( !expr.ok() )
		{
			return expr;
		}

		stmt = expr.unwrap();

		// TODO: Better way to do this?
		goto no_semi;
	}
	break;
	case TokenType::open_curly:
	{
		auto expr = parse_block();
		if( !expr.ok() )
		{
			return expr;
		}

		stmt = expr.unwrap();

		goto no_semi;
	}
	break;
	default:
	{
		auto expr = parse_expr_statement();
		if( !expr.ok() )
		{
			return expr;
		}

		stmt = expr.unwrap();
	}
	break;
	}

	{
		auto curr_tok = cursor.consume(TokenType::semicolon);
		if( !curr_tok.ok() )
		{
			return ParseError("Expected ';'", curr_tok.as());
		}
	}

no_semi:
	return ast.Stmt(trail.mark(), stmt);
}

ParseResult<ast::AstNode*>
AstGen::parse_if()
{
	auto trail = get_parse_trail();

	auto tok = cursor.consume(TokenType::if_keyword);
	if( !tok.ok() )
	{
		return ParseError("Expected 'if'", tok.as());
	}

	auto open_paren_tok = cursor.consume_if_expected(TokenType::open_paren);

	auto condition = parse_expr();
	if( !condition.ok() )
	{
		return condition;
	}

	if( open_paren_tok.ok() )
	{
		tok = cursor.consume(TokenType::close_paren);
		if( !tok.ok() )
		{
			return ParseError("Expected ')'", tok.as());
		}
	}

	auto then_block = parse_statement();
	if( !then_block.ok() )
	{
		return then_block;
	}

	// Else block is optional
	cursor.consume_if_expected(TokenType::else_keyword);
	if( tok.ok() )
	{
		auto else_block_result = parse_statement();
		if( !else_block_result.ok() )
		{
			return else_block_result;
		}

		return ast.If(
			trail.mark(), condition.unwrap(), then_block.unwrap(), else_block_result.unwrap());
	}
	else
	{
		return ast.If(trail.mark(), condition.unwrap(), then_block.unwrap(), nullptr);
	}
}

ParseResult<ast::AstNode*>
AstGen::parse_for()
{
	auto trail = get_parse_trail();

	auto tok = cursor.consume(TokenType::for_keyword);
	if( !tok.ok() )
	{
		return ParseError("Expected 'for'", tok.as());
	}

	tok = cursor.consume(TokenType::open_paren);
	if( !tok.ok() )
	{
		return ParseError("Expected '('", tok.as());
	}

	auto init = parse_statement();
	if( !init.ok() )
	{
		return init;
	}

	auto condition = parse_expr();
	if( !condition.ok() )
	{
		return condition;
	}

	tok = cursor.consume(TokenType::semicolon);
	if( !tok.ok() )
	{
		return ParseError("Expected ';'", tok.as());
	}

	auto end_loop = parse_expr_statement();
	if( !end_loop.ok() )
	{
		return end_loop;
	}

	tok = cursor.consume(TokenType::close_paren);
	if( !tok.ok() )
	{
		return ParseError("Expected ';'", tok.as());
	}

	auto body = parse_statement();
	if( !body.ok() )
	{
		return body;
	}

	return ast.For(
		trail.mark(), init.unwrap(), condition.unwrap(), end_loop.unwrap(), body.unwrap());
}

ParseResult<ast::AstNode*>
AstGen::parse_literal()
{
	auto trail = get_parse_trail();

	auto tok = cursor.consume(TokenType::literal);
	if( !tok.ok() )
	{
		return ParseError("Expected literal", tok.as());
	}

	auto curr_tok = tok.unwrap();
	switch( curr_tok.literal_type )
	{
	case LiteralType::integer:
	{
		auto sz = ast.create_string(curr_tok.start, curr_tok.size);
		int val = std::stoi(*sz);
		return ast.NumberLiteral(trail.mark(), val);
	}
	break;
	case LiteralType::string:
	{
		auto sz = ast.create_string(curr_tok.start + 1, curr_tok.size - 2);

		return ast.StringLiteral(trail.mark(), sz);
	}
	break;
	default:
		return ParseError("Expected literal type", tok.as());
	}
}

ParseResult<ast::AstNode*>
AstGen::parse_type_identifier()
{
	auto trail = get_parse_trail();

	auto tok = cursor.consume(TokenType::identifier);
	if( !tok.ok() )
	{
		return ParseError("Expected identifier", tok.as());
	}

	auto tok_val = tok.as();
	auto name = ast.create_string(tok_val.start, tok_val.size);

	return ast.TypeId(trail.mark(), name);
}

ParseResult<ast::AstNode*>
AstGen::parse_identifier()
{
	auto trail = get_parse_trail();

	auto tok = cursor.consume(TokenType::identifier);
	if( !tok.ok() )
	{
		return ParseError("Expected identifier", tok.as());
	}

	auto tok_val = tok.as();
	auto name = ast.create_string(tok_val.start, tok_val.size);

	return ast.ValueId(trail.mark(), name);
}

ParseResult<ast::AstNode*>
AstGen::parse_value_list()
{
	auto trail = get_parse_trail();

	auto args = ast.create_list();

	Token curr_tok = cursor.peek();
	while( curr_tok.type != TokenType::close_paren )
	{
		auto expr = parse_expr();
		if( !expr.ok() )
		{
			return expr;
		}

		args->append(expr.unwrap());

		// Also catches trailing comma.
		cursor.consume_if_expected(TokenType::comma);
		curr_tok = cursor.peek();
	}

	return ast.ExprList(trail.mark(), args);
}

ParseResult<ast::AstNode*>
AstGen::parse_call(ast::AstNode* base)
{
	auto trail = get_parse_trail();

	auto tok = cursor.consume(TokenType::open_paren);
	if( !tok.ok() )
	{
		return ParseError("Expected '('", tok.as());
	}

	auto args = parse_value_list();
	if( !args.ok() )
	{
		return args;
	}

	tok = cursor.consume(TokenType::close_paren);
	if( !tok.ok() )
	{
		return ParseError("Expected ')'", tok.as());
	}

	return ast.FnCall(trail.mark(), base, args.unwrap());
}

ParseResult<ast::AstNode*>
AstGen::parse_member_reference(ast::AstNode* base)
{
	auto trail = get_parse_trail();

	auto tok = cursor.consume(TokenType::dot);
	if( !tok.ok() )
	{
		return ParseError("Expected '.'", tok.as());
	}

	auto identifier = parse_identifier();
	if( !identifier.ok() )
	{
		return identifier;
	}

	auto member_name = ast.create_string(tok.as().start, tok.as().size);

	return ast.MemberAccess(trail.mark(), base, identifier.unwrap());
}

ParseResult<ast::AstNode*>
AstGen::parse_simple_expr()
{
	auto trail = get_parse_trail();
	AstNode* result = nullptr;
	auto tok = cursor.peek();
	switch( tok.type )
	{
	case TokenType::literal:
	{
		auto expr = parse_literal();
		if( !expr.ok() )
		{
			return expr;
		}
		result = expr.unwrap();
		break;
	}

	case TokenType::identifier:
	{
		auto expr = parse_identifier();
		if( !expr.ok() )
		{
			return expr;
		}
		result = expr.unwrap();
		break;
	}

	case TokenType::open_paren:
	{
		cursor.consume(TokenType::open_paren);
		auto expr = parse_expr();
		if( !expr.ok() )
		{
			return expr;
		}

		auto ctok = cursor.consume(TokenType::close_paren);
		if( !ctok.ok() )
		{
			return ParseError("Expected ')'", ctok.as());
		}

		result = expr.unwrap();
		break;
	}

	default:
		return ParseError("Expected simple expression.", tok);
	}

	return ast.Expr(trail.mark(), result);
}

ParseResult<ast::AstNode*>
AstGen::parse_postfix_expr()
{
	auto trail = get_parse_trail();

	auto expr = parse_simple_expr();
	if( !expr.ok() )
	{
		return expr;
	}

	while( true )
	{
		auto tok = cursor.peek();

		switch( tok.type )
		{
		case TokenType::dot:
			// Member dereference
			expr = parse_member_reference(expr.unwrap());
			if( !expr.ok() )
			{
				return expr;
			}
			expr = ast.Expr(trail.mark(), expr.unwrap());
			break;
		case TokenType::open_paren:
			expr = parse_call(expr.unwrap());
			if( !expr.ok() )
			{
				return expr;
			}
			expr = ast.Expr(trail.mark(), expr.unwrap());
			break;
		default:
			expr = ast.Expr(trail.mark(), expr.unwrap());
			goto done;
			break;
		}
	}
done:
	return expr;
}

ParseResult<ast::AstNode*>
AstGen::parse_expr()
{
	auto trail = get_parse_trail();
	auto lhs = parse_postfix_expr();
	if( !lhs.ok() )
		return lhs;

	auto op = parse_bin_op(0, lhs.unwrap());
	if( !op.ok() )
		return op;

	return ast.Expr(trail.mark(), op.unwrap());
}

ParseResult<ast::AstNode*>
AstGen::parse_non_var_arg_fn_param()
{
	auto param_trail = get_parse_trail();
	auto identifer = parse_identifier();
	if( !identifer.ok() )
	{
		return identifer;
	}

	auto tok = cursor.consume(TokenType::colon);
	if( !tok.ok() )
	{
		return ParseError("Expected ':'", tok.as());
	}

	auto type_decl = parse_type_decl(false);
	if( !type_decl.ok() )
	{
		return type_decl;
	}

	return ast.ValueDecl(param_trail.mark(), identifer.unwrap(), type_decl.unwrap());
}

ParseResult<ast::AstNode*>
AstGen::parse_fn_param()
{
	auto param_trail = get_parse_trail();
	if( cursor.peek().type != TokenType::var_args )
	{
		return parse_non_var_arg_fn_param();
	}
	else
	{
		auto param_trail = get_parse_trail();
		cursor.consume(TokenType::var_args);
		return ast.VarArg(param_trail.mark());
	}
}

ParseResult<ast::AstNode*>
AstGen::parse_function_parameter_list()
{
	auto trail = get_parse_trail();

	auto result = ast.create_list();

	Token curr_tok = cursor.peek();
	while( curr_tok.type != TokenType::close_paren )
	{
		auto decl = parse_fn_param();
		if( !decl.ok() )
			return decl;

		// Also catches trailing comma.
		cursor.consume_if_expected(TokenType::comma);

		result->append(decl.unwrap());
		curr_tok = cursor.peek();
	}

	return ast.FnParamList(trail.mark(), result);
}

ParseResult<ast::AstNode*>
AstGen::parse_function_proto()
{
	auto trail = get_parse_trail();

	auto fn_identifier = parse_type_identifier();
	if( !fn_identifier.ok() )
	{
		return fn_identifier;
	}

	auto tok = cursor.consume(TokenType::open_paren);
	if( !tok.ok() )
	{
		return ParseError("Expected '('", tok.as());
	}

	auto params = parse_function_parameter_list();
	if( !params.ok() )
	{
		return params;
	}

	tok = cursor.consume(TokenType::close_paren);
	if( !tok.ok() )
	{
		return ParseError("Expected ')'", tok.as());
	}

	auto infer_type_tok = cursor.consume_if_expected(TokenType::colon);
	// if( !infer_type_tok.ok() )
	// {
	// 	return ParseError("Expected ':'", infer_type_tok.as());
	// }

	if( infer_type_tok.ok() )
	{
		auto return_type_identifier = parse_type_decl(false);
		if( !return_type_identifier.ok() )
		{
			return return_type_identifier;
		}
		return ast.FnProto(
			trail.mark(), fn_identifier.unwrap(), params.unwrap(), return_type_identifier.unwrap());
	}
	else
	{
		return ast.FnProto(
			trail.mark(), fn_identifier.unwrap(), ast.TypeDeclaratorEmpty(), params.unwrap());
	}
}

ParseResult<AstNode*>
AstGen::parse_extern_function()
{
	auto trail = get_parse_trail();

	auto tok = cursor.consume(TokenType::extern_keyword);
	if( !tok.ok() )
	{
		return ParseError("Expected 'extern'", tok.as());
	}

	tok = cursor.consume(TokenType::fn);
	if( !tok.ok() )
	{
		return ParseError("Expected 'fn'", tok.as());
	}

	auto proto = parse_function_proto();
	if( !proto.ok() )
	{
		return proto;
	}

	if( proto.unwrap()->data.fn_proto.return_type == nullptr )
	{
		return ParseError("Extern functions must specify a return type.", cursor.peek());
	}

	tok = cursor.consume(TokenType::semicolon);
	if( !tok.ok() )
	{
		return ParseError("Expected ';'", tok.as());
	}
	return ast.ExternFn(trail.mark(), proto.unwrap());
}

ParseResult<ast::AstNode*>
AstGen::parse_function()
{
	auto trail = get_parse_trail();

	auto tok = cursor.consume(TokenType::fn);
	if( !tok.ok() )
	{
		return ParseError("Expected 'fn'", tok.as());
	}

	auto proto = parse_function_proto();
	if( !proto.ok() )
	{
		return proto;
	}

	auto definition = parse_function_body();
	if( !definition.ok() )
	{
		return definition;
	}

	return ast.Fn(trail.mark(), proto.unwrap(), definition.unwrap());
}

ParseResult<ast::AstNode*>
AstGen::parse_function_body()
{
	//
	return parse_block();
}

ParseTrail
AstGen::get_parse_trail()
{
	return ParseTrail{cursor, meta};
}