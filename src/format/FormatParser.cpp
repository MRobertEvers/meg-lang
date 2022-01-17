#include "FormatParser.h"

#include "ast/ast.h"

FormatParser::GroupScope::GroupScope(FormatParser& ctx)
	: ctx(ctx)
{
	previous = ctx.current_line;

	ctx.current_line = NodeSpan::Group();
}

FormatParser::GroupScope::~GroupScope()
{
	previous.append_span(ctx.current_line);

	ctx.current_line = previous;
}

FormatParser::DocumentScope::DocumentScope(FormatParser& ctx)
	: ctx(ctx)
{
	previous = ctx.current_line;

	ctx.current_line = NodeSpan::Document();
}

FormatParser::DocumentScope::~DocumentScope()
{
	previous.append_span(ctx.current_line);

	ctx.current_line = previous;
}

FormatParser::IndentScope::IndentScope(FormatParser& ctx)
	: ctx(ctx)
{
	previous = ctx.current_line;

	ctx.current_line = NodeSpan::Indent();
}

FormatParser::IndentScope::~IndentScope()
{
	previous.append_span(ctx.current_line);

	ctx.current_line = previous;
}

void
FormatParser::visit(ast::Module const* node)
{
	DocumentScope scope{*this};

	for( auto const& s : node->statements )
	{
		s->visit(this);
	}
}

void
FormatParser::visit(ast::Function const* node)
{
	DocumentScope st{*this};
	node->Proto->visit(this);

	node->Body->visit(this);

	// if( source )
	// {
	// 	auto ast_span = node->get_span();
	// 	if( ast_span.size > 0 )
	// 	{
	// 		auto token = source->at(ast_span.start + ast_span.size - 1);
	// 		if( token.num_trailing_newlines != 0 )
	// 		{
	// 			append_span(NodeSpan{"\n"});
	// 		}
	// 	}
	// }
}

void
FormatParser::visit(ast::Block const* node)
{
	DocumentScope scope{*this};

	append_span(NodeSpan{"{"});

	{
		IndentScope s{*this};
		append_span(NodeSpan::HardLine());

		int idx = 0;
		for( auto& stmt : node->statements )
		{
			stmt->visit(this);

			if( idx != node->statements.size() - 1 )
			{
				append_span(NodeSpan::HardLine());
			}
			idx += 1;
		}
	}

	append_span(NodeSpan::HardLine());
	append_span(NodeSpan{"}"});
	append_span(NodeSpan::HardLine());
}

void
FormatParser::visit(ast::BinaryOperation const* node)
{
	node->LHS->visit(this);

	append_span(NodeSpan{" "});
	append_span(NodeSpan{String{node->Op} + " "});
	node->RHS->visit(this);
}

void
FormatParser::visit(ast::Number const* node)
{
	append_span(NodeSpan{std::to_string(node->Val)});
}

void
FormatParser::visit(ast::Return const* node)
{
	append_span(NodeSpan{"return "});

	node->ReturnExpr->visit(this);
	append_span(NodeSpan{";"});
}

void
FormatParser::visit(ast::Prototype const* node)
{
	append_span("fn ");
	node->Name->visit(this);
	append_span("(");

	{
		GroupScope g{*this};
		IndentScope ind{*this};
		append_span(NodeSpan::SoftLine());

		auto& args = node->get_parameters()->Parameters;

		for( int i = 0; i < args.size(); i++ )
		{
			auto& arg = args[i];

			arg->Name->visit(this);

			append_span(": ");

			arg->Type->visit(this);

			if( i != args.size() - 1 )
			{
				append_span(",");
				append_span(NodeSpan::Line());
			}
		}
	}

	append_span("): ");
	node->ReturnType->visit(this);
	append_span(" ");
}

void
FormatParser::visit(ast::ValueIdentifier const* node)
{
	append_span(NodeSpan{node->get_name()});
}

void
FormatParser::visit(ast::TypeIdentifier const* node)
{
	append_span(NodeSpan{node->get_name()});
}

void
FormatParser::visit(ast::Let const* node)
{
	append_span(NodeSpan{"let "});

	node->Name->visit(this);
	if( !node->Type->is_empty() )
	{
		append_span(NodeSpan{": "});
		node->Type->visit(this);
	}

	append_span(NodeSpan{" ="});
	append_span(NodeSpan::Line());
	{
		IndentScope ind{*this};
		node->RHS->visit(this);
		append_span(NodeSpan{";"});
	}
}

void
FormatParser::visit(ast::Struct const* node)
{
	DocumentScope st{*this};
	append_span(NodeSpan{"struct "});

	node->TypeName->visit(this);
	append_span(NodeSpan{" {"});

	{
		GroupScope g{*this};
		IndentScope s{*this};

		append_span(NodeSpan::HardLine());

		int idx = 0;
		for( auto& m : node->MemberVariables )
		{
			m->Name->visit(this);

			append_span(NodeSpan{": "});

			m->Type->visit(this);

			append_span(NodeSpan{";"});
			if( idx != node->MemberVariables.size() - 1 )
			{
				append_span(NodeSpan::HardLine());
			}

			idx += 1;
		}
	}

	append_span(NodeSpan::HardLine());
	append_span(NodeSpan{"}"});
	append_span(NodeSpan::HardLine());

	append_newline_if_source(node);
}

void
FormatParser::visit(ast::MemberReference const* node)
{
	node->base->visit(this);
	append_span(NodeSpan{"."});
	node->name->visit(this);
}

void
FormatParser::visit(ast::TypeDeclarator const* node)
{
	auto base = node->get_base();
	if( base )
	{
		base->visit(this);
	}
	append_span(NodeSpan{node->get_name()});
}

void
FormatParser::visit(ast::If const* node)
{
	append_span(NodeSpan{"if "});

	append_span("(");
	{
		GroupScope g{*this};
		IndentScope ind{*this};
		append_span(NodeSpan::SoftLine());
		node->condition->visit(this);
		append_span(NodeSpan::SoftLine());
	}
	append_span(")");
	append_span(NodeSpan{" "});
	node->then_block->visit(this);

	if( !node->else_block.is_null() )
	{
		append_span(NodeSpan{"else "});

		node->else_block->visit(this);
	}
}

void
FormatParser::visit(ast::Assign const* node)
{
	node->lhs->visit(this);
	append_span(NodeSpan{" ="});
	append_span(NodeSpan::Line());

	node->rhs->visit(this);
	append_span(NodeSpan{";"});
}

void
FormatParser::visit(ast::While const* node)
{
	append_span(NodeSpan{"while "});

	node->condition->visit(this);
	append_span(NodeSpan{" "});
	node->loop_block->visit(this);
}

void
FormatParser::visit(ast::Call const* node)
{
	node->call_target->visit(this);
	append_span(NodeSpan{"("});

	{
		IndentScope indent{*this};
		append_span(NodeSpan::SoftLine());

		auto& args = node->args.args;
		for( int i = 0; i < args.size(); i++ )
		{
			auto& arg = args[i];
			arg->visit(this);

			if( i != args.size() - 1 )
			{
				append_span(NodeSpan{","});
				append_span(NodeSpan::Line());
			}
		}
	}

	append_span(NodeSpan{")"});
}

void
FormatParser::visit(ast::Statement const* node)
{
	node->stmt->visit(this);

	append_newline_if_source(node);
}

void
FormatParser::visit(ast::Expression const* node)
{
	node->base->visit(this);
}

void
FormatParser::append_span(NodeSpan span)
{
	current_line.append_span(span);
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
{}

NodeSpan
FormatParser::get_span(Vec<Token> const* source, ast::IAstNode const* node)
{
	FormatParser g{source};

	node->visit(&g);

	return g.current_line.children[0];
}