

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

static std::string
to_simple(AstId& id)
{
	assert(id.kind == AstId::IdKind::Simple);

	return id.name_parts.parts[0];
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
Parser::parse_name_parts(AstId::IdKind mode)
{
	std::vector<std::string> name_parts;
	auto tok = cursor.consume(TokenKind::Identifier);
	if( !tok.ok() )
		return ParseError("Expected identifier", tok.token());

	Token tok_val = tok.token();
	name_parts.emplace_back(to_string(tok_val));

	if( mode == AstId::IdKind::Simple )
		return name_parts;

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
Parser::parse_decl_list()
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
		Token token = cursor.peek();
		switch( token.kind )
		{
		case TokenKind::TemplateKw:
		{
			ParseResult<AstNode*> statement = parse_template();
			if( !statement.ok() )
				return statement;

			statements.push_back(statement.unwrap());
			break;
		}
		default:
		{
			ParseResult<AstNode*> statement = parse_module_statement();
			if( !statement.ok() )
				return statement;

			statements.push_back(statement.unwrap());
		}
		break;
		}
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
		return parse_func();
	case TokenKind::StructKw:
		return parse_struct();
	case TokenKind::UnionKw:
		return parse_union();
	case TokenKind::EnumKw:
		return parse_enum();
	case TokenKind::InterfaceKw:
		return parse_interface();
	default:
		return ParseError("Expected top level 'fn' or 'struct' declaration.");
	}

	return ParseError("??.");
}

ParseResult<AstNode*>
Parser::parse_template()
{
	auto tokc = cursor.consume_if_expected(TokenKind::TemplateKw);
	if( !tokc.ok() )
		return ParseError("Expected 'template'", tokc.token());

	tokc = cursor.consume(TokenKind::Lt);
	if( !tokc.ok() )
		return ParseError("Expected '<'", tokc.token());

	std::vector<AstNode*> types;
	Token tok = cursor.peek();
	while( tok.kind != TokenKind::Gt )
	{
		tokc = cursor.consume(TokenKind::TypenameKw);
		if( !tokc.ok() )
			return ParseError("Expected 'typename'", tokc.token());

		auto typename_result = parse_identifier(AstId::IdKind::Simple);
		if( !typename_result.ok() )
			return typename_result;

		types.push_back(typename_result.unwrap());

		cursor.consume_if_expected(TokenKind::Comma);

		tok = cursor.peek();
	}

	tokc = cursor.consume(TokenKind::Gt);
	if( !tokc.ok() )
		return ParseError("Expected '>'", tokc.token());

	auto template_result = parse_module_statement();
	if( !template_result.ok() )
		return template_result;

	AstNode* template_nod = template_result.unwrap();

	return ast.create<AstTemplate>(Span(), types, template_nod);
}

ParseResult<AstNode*>
Parser::parse_identifier(AstId::IdKind mode)
{
	// auto trail = get_parse_trail();

	auto name_parts = parse_name_parts(mode);
	if( !name_parts.ok() )
		return ParseError(*name_parts.unwrap_error().get());

	AstNode* ast_id = ast.create<AstId>(Span(), name_parts.unwrap(), mode);

	/**
	 * Some research on this expression parsing mode.
	 *
	 * CPP's parser is keeps track of types during parsing,
	 * and will parse an identifier based on the type during parsing.
	 *
	 * This parsing mode resembles more so how Typescript parses
	 * the construct. Basically, if it can be parsed as a
	 * construct like a<b, c>(d), it will. Typescript additionally,
	 * uses speculative parsing, where it will look ahead to see if
	 * it matches the speculative parsing, and if so, will favor
	 * the 'template' identifier mode, otherwise, it will roll back
	 * the parser.
	 */
	auto template_parse_result = parse_template_identifer(ast_id);
	if( template_parse_result.ok() )
		return template_parse_result;
	else
		return ast_id;
}

ParseResult<AstNode*>
Parser::parse_type_decl(bool allow_empty)
{
	// auto trail = get_parse_trail();

	Token tok = cursor.peek();
	if( tok.kind != TokenKind::Identifier )
		return ParseError("Unexpected token while parsing type.", tok);

	auto id_result = parse_identifier(AstId::IdKind::Qualified);
	if( !id_result.ok() )
		return id_result;

	std::vector<AstNode*> types;
	auto tokc = cursor.consume_if_expected(TokenKind::Lt);
	if( tokc.ok() )
	{
		tok = cursor.peek();
		while( tok.kind != TokenKind::Gt )
		{
			auto typename_result = parse_type_decl(false);
			if( !typename_result.ok() )
				return typename_result;

			types.push_back(typename_result.unwrap());

			cursor.consume_if_expected(TokenKind::Comma);

			tok = cursor.peek();
		}

		tokc = cursor.consume(TokenKind::Gt);
		if( !tokc.ok() )
			return ParseError("Expected '>'", tokc.token());
	}

	int indirection = 0;
	tokc = cursor.consume_if_expected(TokenKind::Star);
	for( ; tokc.ok(); indirection++ )
		tokc = cursor.consume_if_expected(TokenKind::Star);

	return ast.create<AstTypeDeclarator>(Span(), types, id_result.unwrap(), indirection);
}

ParseResult<AstNode*>
Parser::parse_var_decl(bool allow_untyped)
{
	auto id = parse_identifier(AstId::IdKind::Simple);
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

class RestoreCursor
{
	Cursor& ref;
	int save;
	bool reset = true;

public:
	RestoreCursor(Cursor& ref)
		: ref(ref)
		, save(ref.save_point()){};
	~RestoreCursor()
	{
		if( reset )
			ref.reset_point(save);
	}
	void release() { reset = false; }
};

ParseResult<AstNode*>
Parser::parse_template_identifer(AstNode* base)
{
	RestoreCursor restore(cursor);

	ConsumeResult tokc = cursor.consume(TokenKind::Lt);
	if( !tokc.ok() )
		return ParseError("Expected '<'", tokc.token());

	// Look ahead to see if there is a closing '>'
	// The idea is that this should be parsed as
	// a template id my_func<id>(4) instead of
	// my_func < id > (4) because expr < expr > expr
	// is never valid.
	std::vector<AstNode*> types;
	Token tok = cursor.peek();
	while( tok.kind != TokenKind::Gt )
	{
		auto expr_result = parse_type_decl(false);
		if( !expr_result.ok() )
			return expr_result;

		types.push_back(expr_result.unwrap());

		// Also catches trailing comma.
		cursor.consume_if_expected(TokenKind::Comma);
		tok = cursor.peek();
	}

	tokc = cursor.consume(TokenKind::Gt);
	if( !tokc.ok() )
		return ParseError("Expected '>'", tokc.token());

	restore.release();
	return ast.create<AstTemplateId>(Span(), types, base);
}

ParseResult<AstNode*>
Parser::parse_indirect_member_access(AstNode* base)
{
	ConsumeResult tok = cursor.consume(TokenKind::SkinnyArrow);
	if( !tok.ok() )
		return ParseError("Expected '->'", tok.token());

	auto expr_result = parse_identifier(AstId::IdKind::Simple);
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

	auto expr_result = parse_identifier(AstId::IdKind::Simple);
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
Parser::parse_assign(AstNode* lhs)
{
	ConsumeResult tokc = cursor.consume({
		TokenKind::Eq,
	});
	if( !tokc.ok() )
		return ParseError("Expected 'assignment'", tokc.token());

	auto expr_result = parse_expr();
	if( !expr_result.ok() )
		return expr_result;

	return ast.create<AstAssign>(Span(), lhs, expr_result.unwrap());
}

ParseResult<AstNode*>
Parser::parse_discriminating_block()
{
	ConsumeResult tokc = cursor.consume(TokenKind::FatArrow);
	if( !tokc.ok() )
		return ParseError("Expected '=>'", tokc.token());

	tokc = cursor.consume(TokenKind::OpenParen);
	if( !tokc.ok() )
		return ParseError("Expected '('", tokc.token());

	auto parameters = parse_decl_list();
	if( !parameters.ok() )
		return ParseError("Failed parsing func params");

	tokc = cursor.consume(TokenKind::CloseParen);
	if( !tokc.ok() )
		return ParseError("Expected ')'", tokc.token());

	auto block_result = parse_block();
	if( !block_result.ok() )
		return block_result;

	return ast.create<AstDiscriminatingBlock>(Span(), parameters.unwrap(), block_result.unwrap());
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
Parser::parse_func()
{
	// auto trail = get_parse_trail();

	auto proto = parse_func_proto();
	if( !proto.ok() )
		return proto;

	auto definition = parse_func_body();
	if( !definition.ok() )
		return definition;

	return ast.create<AstFunc>(Span(), proto.unwrap(), definition.unwrap());
}

ParseResult<AstNode*>
Parser::parse_func_proto()
{
	// auto trail = get_parse_trail();
	auto tokc = cursor.consume(TokenKind::FnKw);
	if( !tokc.ok() )
		return ParseError("Expected 'fn'", tokc.token());

	auto id_result = parse_identifier(AstId::IdKind::Simple);
	if( !id_result.ok() )
		return id_result;

	tokc = cursor.consume(TokenKind::OpenParen);
	if( !tokc.ok() )
		return ParseError("Expected '('", tokc.token());

	auto parameters = parse_decl_list();
	if( !parameters.ok() )
		return ParseError("Failed parsing func params");

	tokc = cursor.consume(TokenKind::CloseParen);
	if( !tokc.ok() )
		return ParseError("Expected ')'", tokc.token());

	tokc = cursor.consume(TokenKind::Colon);
	if( !tokc.ok() )
		return ParseError("Expected ':'", tokc.token());

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
}

ParseResult<AstNode*>
Parser::parse_func_body()
{
	return parse_block();
}

ParseResult<AstNode*>
Parser::parse_while()
{
	auto tok = cursor.consume(TokenKind::WhileKw);
	if( !tok.ok() )
		return ParseError("Expected 'while'", tok.token());

	auto cond_result = parse_expr();
	if( !cond_result.ok() )
		return cond_result;

	auto body_result = parse_statement();
	if( !body_result.ok() )
		return body_result;

	return ast.create<AstWhile>(Span(), cond_result.unwrap(), body_result.unwrap());
}

ParseResult<AstNode*>
Parser::parse_for()
{
	auto tokc = cursor.consume(TokenKind::ForKw);
	if( !tokc.ok() )
		return ParseError("Expected 'for'", tokc.token());

	tokc = cursor.consume(TokenKind::OpenParen);
	if( !tokc.ok() )
		return ParseError("Expected '('", tokc.token());

	std::vector<AstNode*> inits;
	auto tok = cursor.peek();
	while( tok.kind != TokenKind::SemiColon )
	{
		switch( tok.kind )
		{
		case TokenKind::LetKw:
		{
			auto let_result = parse_let();
			if( !let_result.ok() )
				return let_result;
			inits.push_back(ast.create<AstStmt>(Span(), let_result.unwrap()));
			break;
		}
		default:
		{
			auto expr_result = parse_expr_stmt();
			if( !expr_result.ok() )
				return expr_result;
			inits.push_back(ast.create<AstStmt>(Span(), expr_result.unwrap()));
			break;
		}
		}

		cursor.consume_if_expected(TokenKind::Comma);

		tok = cursor.peek();
	}

	cursor.consume_if_expected(TokenKind::SemiColon);

	AstNode* cond = nullptr;
	tok = cursor.peek();
	if( tok.kind != TokenKind::SemiColon )
	{
		auto cond_result = parse_expr();
		if( !cond_result.ok() )
			return cond_result;

		cond = cond_result.unwrap();
	}
	cursor.consume_if_expected(TokenKind::SemiColon);

	std::vector<AstNode*> afters;
	tok = cursor.peek();
	while( tok.kind != TokenKind::CloseParen )
	{
		auto expr_result = parse_expr_stmt();
		if( !expr_result.ok() )
			return expr_result;

		afters.push_back(ast.create<AstStmt>(Span(), expr_result.unwrap()));

		cursor.consume_if_expected(TokenKind::Comma);

		tok = cursor.peek();
	}

	cursor.consume_if_expected(TokenKind::CloseParen);

	auto body_result = parse_statement();
	if( !body_result.ok() )
		return body_result;

	return ast.create<AstFor>(Span(), inits, cond, afters, body_result.unwrap());
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
Parser::parse_switch()
{
	auto tok = cursor.consume(TokenKind::SwitchKw);
	if( !tok.ok() )
		return ParseError("Expected 'struct'", tok.token());

	auto cond_result = parse_expr();
	if( !cond_result.ok() )
		return cond_result;

	tok = cursor.consume(TokenKind::OpenCurly);
	if( !tok.ok() )
		return ParseError("Expected '{'", tok.token());

	std::vector<AstNode*> branches;
	auto curr_tok = cursor.peek();
	while( curr_tok.kind != TokenKind::CloseCurly )
	{
		AstNode* branch;
		switch( curr_tok.kind )
		{
		case TokenKind::CaseKw:
		{
			auto case_stmt = parse_case();
			if( !case_stmt.ok() )
				return case_stmt;
			branch = case_stmt.unwrap();
			break;
		}
		case TokenKind::DefaultKw:
		{
			auto default_stmt = parse_default();
			if( !default_stmt.ok() )
				return default_stmt;
			branch = default_stmt.unwrap();
			break;
		}
		default:
			return ParseError("Stray statement", curr_tok);
		}

		branches.push_back(branch);

		curr_tok = cursor.peek();
	}

	cursor.consume(TokenKind::CloseCurly);

	AstNode* ast_switch = ast.create<AstSwitch>(Span(), cond_result.unwrap(), branches);
	return ast_switch;
}

ParseResult<AstNode*>
Parser::parse_case()
{
	auto tokc = cursor.consume(TokenKind::CaseKw);
	if( !tokc.ok() )
		return ParseError("Expected 'case'", tokc.token());

	Token tok = cursor.peek();
	AstNode* cond = nullptr;
	switch( tok.kind )
	{
	case TokenKind::NumberLiteral:
	{
		auto nl_result = parse_number_literal();
		if( !nl_result.ok() )
			return nl_result;
		cond = nl_result.unwrap();
		break;
	}
	case TokenKind::Identifier:
	{
		auto id_result = parse_type_decl(false);
		if( !id_result.ok() )
			return id_result;
		cond = id_result.unwrap();
		break;
	}
	default:
		return ParseError("Disallowed expression in case statement.");
	}

	cursor.consume_if_expected(TokenKind::Colon);

	AstNode* stmt = nullptr;
	tok = cursor.peek();
	switch( tok.kind )
	{
	case TokenKind::FatArrow:
	{
		auto discriminating_result = parse_discriminating_block();
		if( !discriminating_result.ok() )
			return discriminating_result;
		stmt = discriminating_result.unwrap();
		break;
	}
	default:
	{
		auto block_result = parse_statement();
		if( !block_result.ok() )
			return block_result;
		stmt = block_result.unwrap();
		break;
	}
	}

	return ast.create<AstCase>(Span(), cond, stmt);
}

ParseResult<AstNode*>
Parser::parse_default()
{
	auto tok = cursor.consume(TokenKind::DefaultKw);
	if( !tok.ok() )
		return ParseError("Expected 'default'", tok.token());

	cursor.consume_if_expected(TokenKind::Colon);

	auto block_result = parse_statement();
	if( !block_result.ok() )
		return block_result;

	return ast.create<AstDefault>(Span(), block_result.unwrap());
}

ParseResult<AstNode*>
Parser::parse_break()
{
	auto tok = cursor.consume(TokenKind::BreakKw);
	if( !tok.ok() )
		return ParseError("Expected 'break'", tok.token());

	return ast.create<AstBreak>(Span());
}

ParseResult<AstNode*>
Parser::parse_interface()
{
	std::vector<AstNode*> members;

	auto tokc = cursor.consume(TokenKind::InterfaceKw);
	if( !tokc.ok() )
		return ParseError("Expected 'interface'", tokc.token());

	auto id_result = parse_identifier(AstId::IdKind::Simple);
	if( !id_result.ok() )
		return id_result;

	tokc = cursor.consume(TokenKind::OpenCurly);
	if( !tokc.ok() )
		return ParseError("Expected '{'", tokc.token());

	Token tok = cursor.peek();
	while( tok.kind != TokenKind::CloseCurly )
	{
		auto expr_result = parse_func_proto();
		if( !expr_result.ok() )
			return expr_result;

		members.push_back(expr_result.unwrap());

		// Also catches trailing semicolon.
		cursor.consume_if_expected(TokenKind::SemiColon);
		tok = cursor.peek();
	}

	tokc = cursor.consume(TokenKind::CloseCurly);
	if( !tokc.ok() )
		return ParseError("Expected '}'", tokc.token());

	return ast.create<AstInterface>(Span(), id_result.unwrap(), members);
}

ParseResult<AstNode*>
Parser::parse_struct()
{
	std::vector<AstNode*> members;

	auto tok = cursor.consume(TokenKind::StructKw);
	if( !tok.ok() )
		return ParseError("Expected 'struct'", tok.token());

	auto id_result = parse_identifier(AstId::IdKind::Simple);
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

	auto id_result = parse_identifier(AstId::IdKind::Simple);
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

	auto id_result = parse_identifier(AstId::IdKind::Simple);
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
	auto id_result = parse_identifier(AstId::IdKind::Simple);
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
	auto tokc = cursor.consume(TokenKind::IfKw);
	if( !tokc.ok() )
		return ParseError("Expected 'if'", tokc.token());

	auto expr_result = parse_expr();
	if( !expr_result.ok() )
		return expr_result;

	AstNode* then = nullptr;
	Token tok = cursor.peek();
	switch( tok.kind )
	{
	case TokenKind::FatArrow:
	{
		auto discriminating_result = parse_discriminating_block();
		if( !discriminating_result.ok() )
			return discriminating_result;
		then = discriminating_result.unwrap();
		break;
	}
	default:
	{
		auto block_result = parse_statement();
		if( !block_result.ok() )
			return block_result;
		then = block_result.unwrap();
		break;
	}
	}

	tokc = cursor.consume_if_expected(TokenKind::ElseKw);
	if( !tokc.ok() )
		return ast.create<AstIf>(Span(), expr_result.unwrap(), then, nullptr);

	auto else_result = parse_statement();
	if( !else_result.ok() )
		return else_result;

	return ast.create<AstIf>(Span(), expr_result.unwrap(), then, else_result.unwrap());
}

ParseResult<AstNode*>
Parser::parse_is(AstNode* base)
{
	auto tokc = cursor.consume(TokenKind::IsKw);
	if( !tokc.ok() )
		return ParseError("Expected 'is'", tokc.token());

	auto type_decl_result = parse_type_decl(false);
	if( !type_decl_result.ok() )
		return type_decl_result;

	return ast.create<AstIs>(Span(), base, type_decl_result.unwrap());
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

	AstNode* simple_expr = simple_expr_result.unwrap();

	Token tok = cursor.peek();
	switch( tok.kind )
	{
	case TokenKind::OpenSquare:
		return parse_array_access(simple_expr);
	case TokenKind::OpenParen:
		return parse_call(simple_expr);
	case TokenKind::IsKw:
		return parse_is(simple_expr);
	case TokenKind::SkinnyArrow:
		return parse_indirect_member_access(simple_expr);
	case TokenKind::Dot:
		return parse_member_access(simple_expr);
	default:
		return simple_expr;
	}
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
		auto expr_result = parse_identifier(AstId::IdKind::Simple);
		if( !expr_result.ok() )
			return expr_result;
		result = expr_result.unwrap();
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
			TokenKind::AmpAmp,
			TokenKind::PipePipe,
			TokenKind::ExclamEq,
			TokenKind::IsKw //
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
Parser::parse_expr_stmt()
{
	auto expr_result = parse_expr();
	if( !expr_result.ok() )
		return expr_result;

	Token tok = cursor.peek();
	switch( tok.kind )
	{
	case TokenKind::Eq:
		return parse_assign(expr_result.unwrap());
	default:
		break;
	}

	return expr_result.unwrap();
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
}

ParseResult<AstNode*>
Parser::parse_statement()
{
	// auto trail = get_parse_trail();

	auto tok = cursor.peek();
	ConsumeResult tokc(nullptr);

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
	case TokenKind::CaseKw:
	{
		auto case_stmt = parse_case();
		if( !case_stmt.ok() )
			return case_stmt;

		stmt = case_stmt.unwrap();
		goto no_semi;
		break;
	}
	case TokenKind::DefaultKw:
	{
		auto default_stmt = parse_default();
		if( !default_stmt.ok() )
			return default_stmt;

		stmt = default_stmt.unwrap();
		goto no_semi;
		break;
	}
	case TokenKind::BreakKw:
	{
		auto break_stmt = parse_break();
		if( !break_stmt.ok() )
			return break_stmt;

		stmt = break_stmt.unwrap();
		break;
	}
	case TokenKind::WhileKw:
	{
		auto while_stmt = parse_while();
		if( !while_stmt.ok() )
			return while_stmt;

		stmt = while_stmt.unwrap();
		goto no_semi;
		break;
	}
	case TokenKind::ForKw:
	{
		auto for_stmt = parse_for();
		if( !for_stmt.ok() )
			return for_stmt;

		stmt = for_stmt.unwrap();
		goto no_semi;
		break;
	}
	case TokenKind::SwitchKw:
	{
		auto switch_stmt = parse_switch();
		if( !switch_stmt.ok() )
			return switch_stmt;

		stmt = switch_stmt.unwrap();
		goto no_semi;
		break;
	}
	default:
	{
		auto expr_stmt = parse_expr_stmt();
		if( !expr_stmt.ok() )
			return expr_stmt;

		stmt = expr_stmt.unwrap();
		break;
	}
	}

	tokc = cursor.consume(TokenKind::SemiColon);
	if( !tokc.ok() )
		return ParseError("Expected ';'", tokc.token());

no_semi:
	return ast.create<AstStmt>(Span(), stmt);
}