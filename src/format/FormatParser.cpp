#include "FormatParser.h"

#include "ast/ast.h"

void
FormatParser::visit(ast::Module const* node)
{
	OpaqueScope scope{*this, node};

	for( auto const& s : node->statements )
	{
		s->visit(this);
	}
}

void
FormatParser::visit(ast::Function const* node)
{
	OpaqueScope scope{*this, node};

	node->Proto->visit(this);

	node->Body->visit(this);

	if( source )
	{
		auto ast_span = node->get_span();
		if( ast_span.size > 0 )
		{
			auto token = source->at(ast_span.start + ast_span.size - 1);
			if( token.num_trailing_newlines != 0 )
			{
				append_span(NodeSpan{0, "\n"});
			}
		}
	}
}

void
FormatParser::visit(ast::Block const* node)
{
	OpaqueScope scope{*this, node};

	append_span(NodeSpan{0, "{\n"});

	{
		IndentScope s{*this};
		for( auto& stmt : node->statements )
		{
			stmt->visit(this);
		}
	}

	append_span(NodeSpan{0, "}\n"});
}

void
FormatParser::visit(ast::BinaryOperation const* node)
{
	OpaqueScope scope{*this, node};
	node->LHS->visit(this);

	append_span(NodeSpan{0, " "});
	append_span(NodeSpan{0, String{node->Op} + " "});
	node->RHS->visit(this);
}

void
FormatParser::visit(ast::Number const* node)
{
	OpaqueScope scope{*this, node};
	append_span(NodeSpan{0, std::to_string(node->Val)});
}

void
FormatParser::visit(ast::Return const* node)
{
	OpaqueScope scope{*this, node};
	append_span(NodeSpan{0, "return "});

	node->ReturnExpr->visit(this);
	append_span(NodeSpan{0, ";\n"});
}

void
FormatParser::visit(ast::Prototype const* node)
{
	OpaqueScope scope{*this, node, 3};

	append_span(NodeSpan{0, "fn "});
	node->Name->visit(this);
	append_span(NodeSpan{0, "("});

	auto& args = node->get_parameters()->Parameters;

	for( int i = 0; i < args.size(); i++ )
	{
		auto& arg = args[i];

		arg->Name->visit(this);

		append_span(NodeSpan{0, ": "});

		arg->Type->visit(this);

		if( i != args.size() - 1 )
		{
			append_span(NodeSpan{0, ", "});
		}
	}

	append_span(NodeSpan{0, "): "});

	node->ReturnType->visit(this);
}

void
FormatParser::visit(ast::ValueIdentifier const* node)
{
	OpaqueScope scope{*this, node};
	append_span(NodeSpan{0, node->get_name()});
}

void
FormatParser::visit(ast::TypeIdentifier const* node)
{
	OpaqueScope scope{*this, node};
	append_span(NodeSpan{0, node->get_name()});
}

void
FormatParser::visit(ast::Let const* node)
{
	OpaqueScope scope{*this, node, 2};
	append_span(NodeSpan{0, "let "});

	node->Name->visit(this);
	if( !node->Type->is_empty() )
	{
		append_span(NodeSpan{0, ": "});
		node->Type->visit(this);
	}

	append_span(NodeSpan{0, " = "});

	node->RHS->visit(this);

	append_span(NodeSpan{0, ";\n"});
}

void
FormatParser::visit(ast::Struct const* node)
{
	OpaqueScope scope{*this, node};
	append_span(NodeSpan{0, "struct "});

	node->TypeName->visit(this);
	append_span(NodeSpan{0, " {\n"});

	{
		IndentScope s{*this};
		for( auto& m : node->MemberVariables )
		{
			m->Name->visit(this);

			append_span(NodeSpan{0, ": "});

			m->Type->visit(this);

			append_span(NodeSpan{0, ";\n"});
		}
	}

	append_span(NodeSpan{0, "}\n"});

	append_newline_if_source(node);
}

void
FormatParser::visit(ast::MemberReference const* node)
{
	OpaqueScope scope{*this, node, 0};
	node->base->visit(this);
	append_span(NodeSpan{0, "."});
	node->name->visit(this);
}

void
FormatParser::visit(ast::TypeDeclarator const* node)
{
	OpaqueScope scope{*this, node};
	auto base = node->get_base();
	if( base )
	{
		base->visit(this);
	}
	append_span(NodeSpan{0, node->get_name()});
}

void
FormatParser::visit(ast::If const* node)
{
	OpaqueScope scope{*this, node};
	append_span(NodeSpan{0, "if "});

	node->condition->visit(this);
	append_span(NodeSpan{0, " "});
	node->then_block->visit(this);

	if( !node->else_block.is_null() )
	{
		append_span(NodeSpan{0, "else "});

		node->else_block->visit(this);
	}
}

void
FormatParser::visit(ast::Assign const* node)
{
	OpaqueScope scope{*this, node};
	node->lhs->visit(this);
	append_span(NodeSpan{0, " = "});

	node->rhs->visit(this);
	append_span(NodeSpan{0, ";\n"});
}

void
FormatParser::visit(ast::While const* node)
{
	OpaqueScope scope{*this, node};
	append_span(NodeSpan{0, "while "});

	node->condition->visit(this);
	append_span(NodeSpan{0, " "});
	node->loop_block->visit(this);
}

void
FormatParser::visit(ast::Call const* node)
{
	OpaqueScope scope{*this, node, 3};
	node->call_target->visit(this);
	append_span(NodeSpan{0, "("});

	auto& args = node->args.args;
	for( int i = 0; i < args.size(); i++ )
	{
		auto& arg = args[i];
		arg->visit(this);

		if( i != args.size() - 1 )
		{
			append_span(NodeSpan{0, ", "});
		}
	}

	append_span(NodeSpan{0, ")"});
}

void
FormatParser::visit(ast::Statement const* node)
{
	OpaqueScope scope{*this, node};
	node->stmt->visit(this);

	append_newline_if_source(node);
}

void
FormatParser::visit(ast::Expression const* node)
{
	OpaqueScope scope{*this, node};
	node->base->visit(this);
}

void
FormatParser::append_span(NodeSpan span)
{
	printed_source_newline = false;
	if( need_indent )
	{
		current_line.append_span(NodeSpan{0, String(current_indentation * 4, ' ')});
		need_indent = false;
	}

	current_line.append_span(span);

	if( span.is_raw() && span.is_terminator() )
	{
		need_indent = true;
	}
}

/**
 * @brief
 * Depending on where this is called, a new line is already expected to be emitted,
 * so we only need to respect additional source line new lines
 *
 * @param node
 * @param threshold
 */
void
FormatParser::append_newline_if_source(ast::IAstNode const* node, int threshold)
{
	if( source && !printed_source_newline )
	{
		auto ast_span = node->get_span();
		if( ast_span.size > 0 )
		{
			auto token = source->at(ast_span.start + ast_span.size - 1);
			if( token.num_trailing_newlines > threshold )
			{
				append_span(NodeSpan{0, "\n"});
				printed_source_newline = true;
			}
		}
	}
}

NodeSpan
FormatParser::get_span(Vec<Token> const* source, ast::IAstNode const* node, int indentation)
{
	FormatParser g{source};
	g.current_indentation = indentation;
	g.need_indent = indentation != 0;
	node->visit(&g);

	return g.current_line.children[0];
}