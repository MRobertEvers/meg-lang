#include "Parser.h"

#include "bin_op.h"
#include "common/Vec.h"
#include "common/unreachable.h"

#include <map>

using namespace parser;

static ValueIdentifier
to_value_identifier(ConsumeResult const& tok_res, Span span)
{
	auto tok = tok_res.unwrap();

	return ValueIdentifier{span, String{tok.start, tok.size}};
}

static TypeIdentifier
to_type_identifier(ConsumeResult const& tok_res, Span span)
{
	auto tok = tok_res.unwrap();

	return TypeIdentifier{span, String{tok.start, tok.size}};
}

Parser::Parser(TokenCursor& cursor)
	: cursor(cursor)
{
	init_bin_op_lookup();
	// Module global scope.
	// current_scope = ParseScope::CreateDefault();
}

ParseResult<ast::Module>
Parser::parse_module()
{
	auto trail = get_parse_trail();

	Vec<OwnPtr<IStatementNode>> nodes;

	while( cursor.has_tokens() && cursor.peek().type != TokenType::eof )
	{
		auto item = parse_module_top_level_item();
		if( !item.ok() )
		{
			return item;
		}
		nodes.emplace_back(item.unwrap());
	}

	return ast::Module{trail.mark(), std::move(nodes)};
}

ParseResult<IStatementNode>
Parser::parse_module_top_level_item()
{
	auto tok = cursor.peek();
	switch( tok.type )
	{
	// Fall through
	case TokenType::extern_keyword:
	case TokenType::fn:
		return parse_function();
	case TokenType::struct_keyword:
		return parse_struct();
	default:
		return ParseError("Expected top level 'fn' or 'struct' declaration.");
	}
}

ParseResult<Let>
Parser::parse_let()
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

	auto type_decl = TypeDeclarator::Empty();
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

	return ast::Let{trail.mark(), identifier.unwrap(), std::move(type_decl), expr.unwrap()};
}

ParseResult<Block>
Parser::parse_block()
{
	auto trail = get_parse_trail();

	Vec<OwnPtr<IStatementNode>> stmts;

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

		stmts.push_back(stmt.unwrap());

		curr_tok = cursor.peek();
	}
	cursor.consume_if_expected(TokenType::close_curly);

	return Block{trail.mark(), std::move(stmts)};
}

// type declarator, not a type declaration. e.g. x: my_type.
ParseResult<TypeDeclarator>
Parser::parse_type_decl(bool allow_empty)
{
	auto trail = get_parse_trail();

	auto tok = cursor.consume_if_expected(TokenType::identifier);
	if( !tok.ok() )
	{
		if( allow_empty )
		{
			return TypeDeclarator::Empty();
		}
		else
		{
			return ParseError("Unexpected token while parsing type.", tok.as());
		}
	}

	auto tok_val = tok.as();
	auto name = String{tok_val.start, tok_val.size};

	auto type = OwnPtr<TypeDeclarator>::of(trail.mark(), name);
	auto star_tok = cursor.consume_if_expected(TokenType::star);
	while( star_tok.ok() )
	{
		type = TypeDeclarator::PointerToTy(trail.mark(), std::move(type));
		star_tok = cursor.consume_if_expected(TokenType::star);
	}

	return std::move(type);
}

ParseResult<Struct>
Parser::parse_struct()
{
	auto trail = get_parse_trail();

	Vec<OwnPtr<ast::MemberVariableDeclaration>> members;

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

	auto struct_name = to_type_identifier(consume_tok, trail.mark());

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

		auto name = to_value_identifier(consume_tok, member_trail.mark());

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

		members.emplace_back(
			ast::MemberVariableDeclaration{member_trail.mark(), name, decl.unwrap()});

		tok = cursor.peek();
	}

	consume_tok = cursor.consume(TokenType::close_curly);

	return Struct{trail.mark(), struct_name, std::move(members)};
}

ParseResult<IExpressionNode>
Parser::parse_bin_op(int ExprPrec, OwnPtr<IExpressionNode> LHS)
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
		int TokPrec = get_token_precedence(op);

		// If this is a binop that binds at least as tightly as the current binop,
		// consume it, otherwise we are done.
		if( TokPrec < ExprPrec )
			return LHS;

		// Parse the primary expression after the binary operator.
		auto RHS = parse_postfix_expr();
		if( !RHS.ok() )
			return RHS;

		// If BinOp binds less tightly with RHS than the operator after RHS, let
		// the pending operator take RHS as its LHS.
		cur = cursor.peek();
		token_type = tok.as().type;
		op = get_bin_op_from_token_type(token_type);
		int NextPrec = get_token_precedence(op);
		if( TokPrec < NextPrec )
		{
			RHS = parse_bin_op(TokPrec + 1, RHS.unwrap());
			if( !RHS.ok() )
				return RHS;
		}

		// Merge LHS/RHS.
		LHS = BinaryOperation{trail.mark(), op, std::move(LHS), RHS.unwrap()};
	}
}

ParseResult<Assign>
Parser::parse_assign(OwnPtr<IExpressionNode> lhs)
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

	return Assign{trail.mark(), op, std::move(lhs), rhs.unwrap()};
}

/**
 * @brief Does not consume ';'.
 *
 * @return ParseResult<IStatementNode>
 */
ParseResult<IStatementNode>
Parser::parse_expr_statement()
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

ParseResult<While>
Parser::parse_while()
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

	return While{trail.mark(), condition.unwrap(), loop_block.unwrap()};
}

ParseResult<IStatementNode>
Parser::parse_statement()
{
	auto trail = get_parse_trail();

	auto tok = cursor.peek();

	OwnPtr<IStatementNode> stmt = nullptr;
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

		stmt = Return{trail.mark(), return_expr.unwrap()};
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
	return Statement{trail.mark(), std::move(stmt)};
}

ParseResult<If>
Parser::parse_if()
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
	OwnPtr<IStatementNode> else_block = Block{};
	tok = cursor.consume_if_expected(TokenType::else_keyword);
	if( tok.ok() )
	{
		auto else_block_result = parse_statement();
		if( !else_block_result.ok() )
		{
			return else_block_result;
		}

		return If{
			trail.mark(), condition.unwrap(), then_block.unwrap(), else_block_result.unwrap()};
	}
	else
	{
		return If{trail.mark(), condition.unwrap(), then_block.unwrap()};
	}
}

ParseResult<For>
Parser::parse_for()
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

	return ast::For(
		trail.mark(), init.unwrap(), condition.unwrap(), end_loop.unwrap(), body.unwrap());
}

ParseResult<IExpressionNode>
Parser::parse_literal()
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
		auto sz = String{curr_tok.start, curr_tok.size};
		int val = std::stoi(sz);
		return Number(trail.mark(), val);
	}
	break;
	case LiteralType::string:
	{
		auto sz = String{curr_tok.start + 1, curr_tok.size - 2};

		return StringLiteral(trail.mark(), sz);
	}
	break;
	default:
		return ParseError("Expected literal type", tok.as());
	}
}

ParseResult<TypeIdentifier>
Parser::parse_type_identifier()
{
	auto trail = get_parse_trail();

	auto tok = cursor.consume(TokenType::identifier);
	if( !tok.ok() )
	{
		return ParseError("Expected identifier", tok.as());
	}

	auto tok_val = tok.as();
	auto name = String{tok_val.start, tok_val.size};

	return TypeIdentifier(trail.mark(), name);
}

ParseResult<ValueIdentifier>
Parser::parse_identifier()
{
	auto trail = get_parse_trail();

	auto tok = cursor.consume(TokenType::identifier);
	if( !tok.ok() )
	{
		return ParseError("Expected identifier", tok.as());
	}

	auto tok_val = tok.as();
	auto name = String{tok_val.start, tok_val.size};

	return ValueIdentifier(trail.mark(), name);
}

ParseResult<ArgumentList>
Parser::parse_value_list()
{
	auto trail = get_parse_trail();

	Vec<OwnPtr<IExpressionNode>> result;

	Token curr_tok = cursor.peek();
	while( curr_tok.type != TokenType::close_paren )
	{
		auto expr = parse_expr();
		if( !expr.ok() )
		{
			return expr;
		}

		result.emplace_back(expr.unwrap());

		// Also catches trailing comma.
		cursor.consume_if_expected(TokenType::comma);
		curr_tok = cursor.peek();
	}

	return ArgumentList{trail.mark(), result};
}

ParseResult<Call>
Parser::parse_call(OwnPtr<IExpressionNode> base)
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

	auto own = args.unwrap();
	auto own_ptr = own.get();
	own.release();
	return Call{trail.mark(), std::move(base), *own_ptr};
}

// TODO: Anything that takes an existing expr pointer by value and might fail will result
// in that node getting deleted... which is not what we want.
ParseResult<MemberReference>
Parser::parse_member_reference(OwnPtr<IExpressionNode> base)
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

	auto member_name = String{tok.as().start, tok.as().size};

	return MemberReference{trail.mark(), std::move(base), identifier.unwrap()};
}

ParseResult<IExpressionNode>
Parser::parse_simple_expr()
{
	auto trail = get_parse_trail();

	auto result = OwnPtr<IExpressionNode>::null();

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

	return Expression{trail.mark(), std::move(result)};
}

ParseResult<IExpressionNode>
Parser::parse_postfix_expr()
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
			break;
		case TokenType::open_paren:
			expr = parse_call(expr.unwrap());
			if( !expr.ok() )
			{
				return expr;
			}
			break;
		default:
			goto done;
			break;
		}
	}
done:
	return Expression{trail.mark(), expr.unwrap()};
}

ParseResult<IExpressionNode>
Parser::parse_expr()
{
	auto trail = get_parse_trail();
	auto LHS = parse_postfix_expr();
	if( !LHS.ok() )
	{
		return LHS;
	}

	auto OP = parse_bin_op(0, LHS.unwrap());
	if( !OP.ok() )
	{
		return OP;
	}

	return Expression{trail.mark(), OP.unwrap()};
}

ParseResult<ParameterList>
Parser::parse_function_parameter_list()
{
	auto trail = get_parse_trail();

	Vec<OwnPtr<ParameterDeclaration>> result;

	Token curr_tok = cursor.peek();
	while( curr_tok.type != TokenType::close_paren )
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

		// Also catches trailing comma.
		cursor.consume_if_expected(TokenType::comma);

		auto decl = ParameterDeclaration{
			param_trail.mark(),
			identifer.unwrap(),
			type_decl.unwrap(),
		};

		result.emplace_back(decl);
		curr_tok = cursor.peek();
	}

	return ParameterList{trail.mark(), result};
}

ParseResult<Prototype>
Parser::parse_function_proto()
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
		return Prototype{
			trail.mark(), fn_identifier.unwrap(), return_type_identifier.unwrap(), params.unwrap()};
	}
	else
	{
		return Prototype{trail.mark(), fn_identifier.unwrap(), nullptr, params.unwrap()};
	}
}

ParseResult<Function>
Parser::parse_function()
{
	auto trail = get_parse_trail();
	auto extern_tok = cursor.consume_if_expected(TokenType::extern_keyword);

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

	if( extern_tok.ok() )
	{
		if( proto.as()->is_infer_return() )
		{
			return ParseError("Extern functions must specify a return type.", cursor.peek());
		}

		auto tok = cursor.consume(TokenType::semicolon);
		if( !tok.ok() )
		{
			return ParseError("Expected ';'", tok.as());
		}
		return ast::Function(trail.mark(), proto.unwrap(), nullptr);
	}
	else
	{
		auto definition = parse_function_body();
		if( !definition.ok() )
		{
			return definition;
		}

		return ast::Function(trail.mark(), proto.unwrap(), definition.unwrap());
	}
}

ParseResult<Block>
Parser::parse_function_body()
{
	return parse_block();
}

ParseTrail
Parser::get_parse_trail()
{
	return ParseTrail{cursor, meta};
}