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

static ValueIdentifier
to_value_identifier(ConsumeResult const& tok_res, Type const& type)
{
	auto tok = tok_res.unwrap();

	return ValueIdentifier{String{tok.start, tok.size}, type};
}

void
ParseScope::add_name(String const& name, Type const* type)
{
	names.insert(std::make_pair(name, type));
}

Type const*
ParseScope::get_type_for_name(String const& name)
{
	auto find_iter = names.find(name);
	if( find_iter == names.end() )
	{
		if( parent != nullptr )
		{
			return parent->get_type_for_name(name);
		}
		else
		{
			return nullptr;
		}
	}
	else
	{
		return find_iter->second;
	}
}

ParseScope*
ParseScope::CreateDefault()
{
	auto base_scope = new ParseScope();

	base_scope->add_name(i8_type.name, &i8_type);
	base_scope->add_name(i16_type.name, &i16_type);
	base_scope->add_name(i32_type.name, &i32_type);
	base_scope->add_name(u8_type.name, &u8_type);
	base_scope->add_name(u16_type.name, &u16_type);
	base_scope->add_name(u32_type.name, &i32_type);

	return base_scope;
}

Parser::Parser(TokenCursor& cursor)
	: cursor(cursor)
{
	BinopPrecedence['<'] = 10;
	BinopPrecedence['+'] = 20;
	BinopPrecedence['-'] = 20;
	BinopPrecedence['*'] = 40; // highest.

	// Module global scope.
	current_scope = ParseScope::CreateDefault();
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
	auto name_tok = tok;

	tok = cursor.consume(TokenType::colon, TokenType::equal);
	if( !tok.ok() )
	{
		return ParseError("Expected identifier or '='", tok.as());
	}

	auto type_decl = TypeDeclarator::Empty();
	if( tok.as().type == TokenType::colon )
	{
		auto type_result = parse_type_decl(true);
		if( !type_result.ok() )
		{
			return type_result;
		}

		type_decl = type_result.unwrap();
	}

	cursor.consume_if_expected(TokenType::equal);
	auto expr_result = parse_expr();
	if( !expr_result.ok() )
	{
		return expr_result;
	}

	auto expr = expr_result.unwrap();
	if( type_decl->get_type().is_infer_type() )
	{
		type_decl = TypeDeclarator{expr->get_type()};
	}
	else
	{
		// TODO: Check that types are the same.
	}

	auto name = to_value_identifier(name_tok, type_decl->get_type());

	current_scope->add_name(name.get_fqn(), &type_decl->get_type());

	return ast::Let{std::move(name), std::move(type_decl), std::move(expr)};
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

// type declarator, not a type declaration. e.g. x: my_type.
ParseResult<TypeDeclarator>
Parser::parse_type_decl(bool allow_empty)
{
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

	auto existing_type = current_scope->get_type_for_name(name);
	if( existing_type == nullptr )
	{
		return ParseError("'" + name + "' is not a type.", tok.as());
	}

	// The pointer-to type is owned

	auto type = OwnPtr<TypeDeclarator>::of(*existing_type);
	auto star_tok = cursor.consume_if_expected(TokenType::star);
	while( star_tok.ok() )
	{
		type = TypeDeclarator::PointerToTy(std::move(type));
		star_tok = cursor.consume_if_expected(TokenType::star);
	}

	// The type declaration is the owning pointer, but this could return either a pointer to type,
	// which is owned, or a reference to the original type which wouldn't be owned.
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

		auto name_tok = consume_tok;

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

		auto& type = decl.as()->get_type();
		auto name = to_value_identifier(name_tok, type);

		consume_tok = cursor.consume(TokenType::semicolon);
		if( !consume_tok.ok() )
		{
			return ParseError("Expected ';'.", consume_tok.as());
		}

		members.emplace_back(ast::MemberVariableDeclaration{name, decl.unwrap()});

		struct_name.add_member(name.get_fqn(), type);

		tok = cursor.peek();
	}

	current_scope->add_name(struct_name.get_fqn(), &struct_name.get_type());

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
			return LHS;

		// This is a binary operation because TokPrec would be less than ExprPrec if
		// the next token was not a bin op (e.g. if statement or so.)
		char Op = *cur.start;

		// TODO: Consume bin op
		cursor.consume({TokenType::plus, TokenType::star, TokenType::minus});

		// Parse the primary expression after the binary operator.
		auto RHS = parse_postfix_expr();
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
		return Number(val, i32_type);
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
	auto tok = cursor.consume(TokenType::identifier);
	if( !tok.ok() )
	{
		return ParseError("Expected identifier", tok.as());
	}

	// tokens.push_back(tok.unwrap());
	// auto curr_tok = cursor.peek();
	// while( curr_tok.type == TokenType::dot )
	// {
	// 	cursor.consume(TokenType::dot);

	// 	tok = cursor.consume(TokenType::identifier);
	// 	if( !tok.ok() )
	// 	{
	// 		return ParseError("Expected identifier", tok.as());
	// 	}
	// 	tokens.push_back(tok.unwrap());

	// 	curr_tok = cursor.peek();
	// }
	auto tok_val = tok.as();
	auto name = String{tok_val.start, tok_val.size};

	auto type = current_scope->get_type_for_name(name);

	if( type == nullptr )
	{
		return ParseError(
			String("Could not find identifier ") + String(name.data()) + " in current scope.",
			tok.as());
	}
	else
	{
		return ValueIdentifier(name, *type);
	}
}

// TODO: Anything that takes an existing expr pointer by value and might fail will result
// in that node getting deleted... which is not what we want.
ParseResult<MemberReference>
Parser::parse_member_reference(OwnPtr<IExpressionNode> base)
{
	auto tok = cursor.consume(TokenType::dot);
	if( !tok.ok() )
	{
		return ParseError("Expected '.'", tok.as());
	}

	tok = cursor.consume(TokenType::identifier);
	if( !tok.ok() )
	{
		return ParseError("Expected identifier", tok.as());
	}

	auto member_name = String{tok.as().start, tok.as().size};
	auto struct_type = &base->get_type();
	// if( struct_type == nullptr )
	// {
	// 	return ParseError(
	// 		"Cannot dereference " + member_name + " from " + struct_type_usage_name.name, tok.as());
	// }

	// TODO: This allows 1 free deref
	if( struct_type->is_pointer_type() )
	{
		struct_type = struct_type->base;
	}

	auto type = struct_type->get_member_type(member_name);
	if( type == nullptr )
	{
		return ParseError(
			"Struct " + struct_type->name + " does not have member " + member_name, tok.as());
	}

	auto identifier = to_value_identifier(tok, *type);

	return MemberReference{std::move(base), identifier, *type};
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
Parser::parse_postfix_expr()
{
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
			// Function call
			break;
		default:
			goto done;
			break;
		}
	}
done:
	return expr;
}

ParseResult<IExpressionNode>
Parser::parse_expr()
{
	auto LHS = parse_postfix_expr();
	if( !LHS.ok() )
	{
		return LHS;
	}

	auto OP = parse_bin_op(0, LHS.unwrap());

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
		auto name = String{name_tok.start, name_tok.size};
		auto type_declarator = type_decl.unwrap();
		current_scope->add_name(name, &type_declarator->get_type());

		auto decl = ParameterDeclaration{
			ValueIdentifier(name, type_declarator->get_type()),
			std::move(type_declarator),
		};

		result.emplace_back(decl);

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
	auto fn_name_decl = TypeIdentifier{String{tok_fn_name.start, tok_fn_name.size}};

	tok = cursor.consume(TokenType::open_paren);
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
	auto type_name = String{tok_fn_return_type.start, tok_fn_return_type.size};
	auto type = current_scope->get_type_for_name(type_name);
	if( type == nullptr )
	{
		return ParseError("Unrecognized typename " + type_name, tok.as());
	}

	auto fn_return_type_decl = TypeDeclarator(*type);

	// This params.unwrap.get is mega shaky. Prototype moves out of the vec so its ok, but damn.
	auto own = params.unwrap();
	auto own_ptr = own.get();
	own.release();
	return Prototype{fn_name_decl, fn_return_type_decl, *own_ptr};
}

ParseResult<Function>
Parser::parse_function()
{
	new_scope();

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

	pop_scope();

	return ast::Function(proto.unwrap(), definition.unwrap());
}

ParseResult<Block>
Parser::parse_function_body()
{
	return parse_block();
}

void
Parser::pop_scope()
{
	auto parent_scope = current_scope->get_parent();
	if( !parent_scope )
	{
		// TODO: Error
	}

	delete current_scope;
	current_scope = parent_scope;
}

void
Parser::new_scope()
{
	ParseScope* scope = new ParseScope(current_scope);

	current_scope = scope;
}