#include "Parser.h"

#include "common/Vec.h"
#include "common/unreachable.h"

#include <map>

// TODO: Dont do this
static std::map<char, int> BinopPrecedence;

static int
get_token_precedence(Token const& token)
{
	// Make sure it's a declared binop.
	int TokPrec = BinopPrecedence[*token.start];
	if( TokPrec <= 0 )
		return -1;
	return TokPrec;
}

Parser::Parser(TokenCursor& cursor)
	: cursor(cursor)
{
	BinopPrecedence['<'] = 10;
	BinopPrecedence['+'] = 20;
	BinopPrecedence['-'] = 20;
	BinopPrecedence['*'] = 40; // highest.
}

ParseResult<ast::Module>
Parser::parse_module()
{
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

	return ast::Module{std::move(nodes)};
}

ParseResult<IStatementNode>
Parser::parse_module_top_level_item()
{
	auto tok = cursor.peek();
	switch( tok.type )
	{
	case TokenType::fn:
	{
		return parse_function();
	}
	case TokenType::struct_keyword:
		return parse_struct();
	default:
		return ParseError("Expected top level 'fn' or 'struct' declaration.");
	}
}

static ValueIdentifier
to_value_identifier(ConsumeResult const& tok_res)
{
	auto tok = tok_res.unwrap();

	return ValueIdentifier{String{tok.start, tok.size}};
}

ParseResult<Let>
Parser::parse_let()
{
	auto tok = cursor.consume(TokenType::let);
	if( !tok.ok() )
	{
		return ParseError("Expected 'let'", tok.as());
	}

	tok = cursor.consume(TokenType::identifier);
	if( !tok.ok() )
	{
		return ParseError("Expected identifier", tok.as());
	}
	auto name = to_value_identifier(tok);

	tok = cursor.consume(TokenType::colon, TokenType::equal);
	if( !tok.ok() )
	{
		return ParseError("Expected identifier or '='", tok.as());
	}

	auto type = parse_type_decl(true);
	if( !type.ok() )
	{
		return type;
	}

	cursor.consume_if_expected(TokenType::equal);
	auto expr = parse_expr();
	if( !expr.ok() )
	{
		return expr;
	}

	return ast::Let{std::move(name), type.unwrap(), expr.unwrap()};
}

ParseResult<Block>
Parser::parse_block()
{
	Vec<OwnPtr<IStatementNode>> stmts;

	auto tok = cursor.consume(TokenType::open_curly);
	if( !tok.ok() )
	{
		return ParseError("Expected '{'", tok.as());
	}

	auto curr_tok = cursor.peek();
	while( curr_tok.type != TokenType::close_curly )
	{
		// TODO: parse statement (i.e. this consumes the semicolon)
		switch( curr_tok.type )
		{
		case TokenType::return_keyword:
		{
			cursor.consume_if_expected(TokenType::return_keyword);

			auto return_expr = parse_expr();
			if( !return_expr.ok() )
			{
				return return_expr;
			}

			stmts.push_back(new Return{return_expr.unwrap()});
			break;
		}
		case TokenType::let:
		{
			auto expr = parse_let();
			if( !expr.ok() )
			{
				return expr;
			}

			stmts.push_back(expr.unwrap());
		}
		break;
		default:
			std::cout << "Expected expression or return statement" << std::endl;
			break;
		}

		tok = cursor.consume(TokenType::semicolon);
		if( !tok.ok() )
		{
			return ParseError("Expected ';'", tok.as());
		}

		curr_tok = cursor.peek();
	}
	cursor.consume_if_expected(TokenType::close_curly);

	return Block{std::move(stmts)};
}

static TypeIdentifier
to_type_identifier(ConsumeResult const& tok_res)
{
	auto tok = tok_res.unwrap();

	return TypeIdentifier{String{tok.start, tok.size}};
}

ParseResult<TypeIdentifier>
Parser::parse_type_decl(bool allow_empty)
{
	auto tok = cursor.consume_if_expected(TokenType::identifier);
	if( !tok.ok() )
	{
		if( allow_empty )
		{
			return TypeIdentifier::Empty();
		}
		else
		{
			return ParseError("Unexpected token while parsing type.", tok.as());
		}
	}

	auto type = OwnPtr<TypeIdentifier>::of(to_type_identifier(tok));

	auto star_tok = cursor.consume_if_expected(TokenType::star);
	while( star_tok.ok() )
	{
		type = TypeIdentifier::PointerToTy(std::move(type));
		star_tok = cursor.consume_if_expected(TokenType::star);
	}

	return std::move(type);
}

ParseResult<Struct>
Parser::parse_struct()
{
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

	auto struct_name = to_type_identifier(consume_tok);

	consume_tok = cursor.consume(TokenType::open_curly);
	if( !consume_tok.ok() )
	{
		return ParseError("Expected block, '{'.", consume_tok.as());
	}

	auto tok = cursor.peek();
	while( tok.type != TokenType::close_curly )
	{
		consume_tok = cursor.consume(TokenType::identifier);
		if( !consume_tok.ok() )
		{
			return ParseError("Expected member declaration.", consume_tok.as());
		}

		auto name = to_value_identifier(consume_tok);

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

		members.emplace_back(ast::MemberVariableDeclaration{name, decl.unwrap()});

		tok = cursor.peek();
	}

	consume_tok = cursor.consume(TokenType::close_curly);

	return Struct{struct_name, std::move(members)};
}

ParseResult<IExpressionNode>
Parser::parse_bin_op(int ExprPrec, OwnPtr<IExpressionNode> LHS)
{
	// If this is a binop, find its precedence.
	while( true )
	{
		auto cur = cursor.peek();
		int TokPrec = get_token_precedence(cur);

		// If this is a binop that binds at least as tightly as the current binop,
		// consume it, otherwise we are done.
		if( TokPrec < ExprPrec )
			return std::move(LHS);

		// This is a binary operation because TokPrec would be less than ExprPrec if
		// the next token was not a bin op (e.g. if statement or so.)
		char Op = *cur.start;

		// TODO: Consume bin op
		cursor.consume(TokenType::struct_keyword);

		// Parse the primary expression after the binary operator.
		auto RHS = parse_simple_expr();
		if( !RHS.ok() )
			return RHS;

		// If BinOp binds less tightly with RHS than the operator after RHS, let
		// the pending operator take RHS as its LHS.
		cur = cursor.peek();
		int NextPrec = get_token_precedence(cur);
		if( TokPrec < NextPrec )
		{
			RHS = parse_bin_op(TokPrec + 1, RHS.unwrap());
			if( !RHS.ok() )
				return RHS;
		}

		// Merge LHS/RHS.
		LHS = BinaryOperation{Op, std::move(LHS), RHS.unwrap()};
	}
}

ParseResult<IExpressionNode>
Parser::parse_literal()
{
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
		return Number(val);
	}
	break;

	default:
		return ParseError("Expected literal type", tok.as());
	}
}

static Path
to_path(Vec<Token>& tokens)
{
	Vec<String> names;
	for( auto& tok : tokens )
	{
		names.emplace_back(tok.start, tok.size);
	}

	return Path{names};
}

ParseResult<ValueIdentifier>
Parser::parse_identifier()
{
	Vec<Token> tokens;

	auto tok = cursor.consume(TokenType::identifier);
	if( !tok.ok() )
	{
		return ParseError("Expected identifier", tok.as());
	}

	tokens.push_back(tok.unwrap());
	auto curr_tok = cursor.peek();
	while( curr_tok.type == TokenType::dot )
	{
		cursor.consume(TokenType::dot);

		tok = cursor.consume(TokenType::identifier);
		if( !tok.ok() )
		{
			return ParseError("Expected identifier", tok.as());
		}
		tokens.push_back(tok.unwrap());

		curr_tok = cursor.peek();
	}

	return ValueIdentifier(to_path(tokens));
}

ParseResult<IExpressionNode>
Parser::parse_simple_expr()
{
	auto tok = cursor.peek();
	switch( tok.type )
	{
	case TokenType::literal:
		return parse_literal();

	case TokenType::identifier:
		return parse_identifier();

	default:
		return ParseError("Expected simple expression.");
	}
}

ParseResult<IExpressionNode>
Parser::parse_expr()
{
	auto LHS = parse_simple_expr();
	if( !LHS.ok() )
	{
		return LHS;
	}

	auto OP = parse_bin_op(0, std::move(LHS.unwrap()));

	return OP;
}
ParseResult<Vec<OwnPtr<ParameterDeclaration>>>
Parser::parse_function_parameter_list()
{
	Vec<OwnPtr<ParameterDeclaration>> result;

	Token curr_tok = cursor.peek();
	while( curr_tok.type != TokenType::close_paren )
	{
		auto tok = cursor.consume(TokenType::identifier);
		if( !tok.ok() )
		{
			return ParseError("Expected identifier", tok.as());
		}

		auto name_tok = tok.as();
		tok = cursor.consume(TokenType::colon);
		if( !tok.ok() )
		{
			return ParseError("Expected ':'", tok.as());
		}

		auto type_decl = parse_type_decl(false);
		if( !type_decl.ok() )
		{
			return type_decl;
		}

		auto decl = ParameterDeclaration{
			ValueIdentifier(String{name_tok.start, name_tok.size}),
			type_decl.unwrap(),
		};

		result.emplace_back(std::move(decl));

		// Also catches trailing comma.
		cursor.consume_if_expected(TokenType::comma);
		curr_tok = cursor.peek();
	}

	return result;
}

ParseResult<Prototype>
Parser::parse_function_proto()
{
	auto tok = cursor.consume(TokenType::identifier);
	if( !tok.ok() )
	{
		return ParseError("Expected identifier", tok.as());
	}

	auto tok_fn_name = tok.unwrap();
	auto fn_name_decl = ValueIdentifier(String{tok_fn_name.start, tok_fn_name.size});

	tok = cursor.consume(TokenType::open_paren);
	if( !tok.ok() )
	{
		return ParseError("Expected '('", tok.as());
	}

	auto params = parse_function_parameter_list();

	tok = cursor.consume(TokenType::close_paren);
	if( !tok.ok() )
	{
		return ParseError("Expected ')'", tok.as());
	}

	tok = cursor.consume(TokenType::colon);
	if( !tok.ok() )
	{
		return ParseError("Expected ':'", tok.as());
	}

	tok = cursor.consume(TokenType::identifier);
	if( !tok.ok() )
	{
		return ParseError("Expected function return type", tok.as());
	}

	auto tok_fn_return_type = tok.unwrap();
	auto fn_return_type_decl =
		TypeIdentifier(String{tok_fn_return_type.start, tok_fn_return_type.size});

	return Prototype{fn_name_decl, fn_return_type_decl, std::move(*params.unwrap().get())};
}

ParseResult<Function>
Parser::parse_function()
{
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

	// TODO: Pass in proto to check return type
	auto definition = parse_function_body();
	if( !definition.ok() )
	{
		return definition;
	}

	return ast::Function(std::move(proto.unwrap()), std::move(definition.unwrap()));
}

ParseResult<Block>
Parser::parse_function_body()
{
	return parse_block();
}
