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
		visit_node(s.get());
	}
}

void
FormatParser::visit(ast::Function const* node)
{
	DocumentScope st{*this};
	visit_node(node->Proto.get());

	visit_node(node->Body.get());

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
			visit_node(stmt.get());

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
	visit_node(node->LHS.get());

	append_span(NodeSpan{" "});
	append_span(NodeSpan{String{node->Op} + " "});

	visit_node(node->RHS.get());
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

	visit_node(node->ReturnExpr.get());
	append_span(NodeSpan{";"});
}

void
FormatParser::visit(ast::Prototype const* node)
{
	append_span("fn ");
	visit_node(node->Name.get());
	append_span("(");

	{
		GroupScope g{*this};
		{
			IndentScope ind{*this};
			append_span(NodeSpan::SoftLine());

			auto& args = node->get_parameters()->Parameters;

			for( int i = 0; i < args.size(); i++ )
			{
				auto& arg = args[i];

				visit_node(arg->Name.get());

				append_span(": ");

				visit_node(arg->Type.get());

				if( i != args.size() - 1 )
				{
					append_span(",");
					append_span(NodeSpan::SoftLine());
				}
			}
		}
		append_span(NodeSpan::SoftLine());
	}

	append_span("): ");
	visit_node(node->ReturnType.get());
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

	visit_node(node->Name.get());
	if( !node->Type->is_empty() )
	{
		append_span(NodeSpan{": "});
		visit_node(node->Type.get());
	}

	append_span(NodeSpan{" ="});
	append_span(NodeSpan::Line());
	{
		IndentScope ind{*this};
		visit_node(node->RHS.get());
		append_span(NodeSpan{";"});
	}
}

void
FormatParser::visit(ast::Struct const* node)
{
	DocumentScope st{*this};
	append_span(NodeSpan{"struct "});

	visit_node(node->TypeName.get());
	append_span(NodeSpan{" {"});

	{
		GroupScope g{*this};
		IndentScope s{*this};

		append_span(NodeSpan::HardLine());

		int idx = 0;
		for( auto& m : node->MemberVariables )
		{
			visit_node(m->Name.get());

			append_span(NodeSpan{": "});

			visit_node(m->Type.get());

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
}

void
FormatParser::visit(ast::MemberReference const* node)
{
	visit_node(node->base.get());
	append_span(NodeSpan{"."});
	visit_node(node->name.get());
}

void
FormatParser::visit(ast::TypeDeclarator const* node)
{
	auto base = node->get_base();
	if( base )
	{
		visit_node(base);
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
		visit_node(node->condition.get());
		append_span(NodeSpan::SoftLine());
	}
	append_span(")");
	append_span(NodeSpan{" "});
	visit_node(node->then_block.get());

	if( !node->else_block.is_null() )
	{
		append_span(NodeSpan{"else "});

		visit_node(node->else_block.get());
	}
}

void
FormatParser::visit(ast::Assign const* node)
{
	visit_node(node->lhs.get());
	append_span(NodeSpan{" ="});
	append_span(NodeSpan::Line());

	visit_node(node->rhs.get());
	append_span(NodeSpan{";"});
}

void
FormatParser::visit(ast::While const* node)
{
	append_span(NodeSpan{"while "});

	visit_node(node->condition.get());
	append_span(NodeSpan{" "});
	visit_node(node->loop_block.get());
}

void
FormatParser::visit(ast::Call const* node)
{
	visit_node(node->call_target.get());

	append_span(NodeSpan{"("});

	{
		GroupScope grp{*this};
		{
			IndentScope indent{*this};
			append_span(NodeSpan::SoftLine());

			auto& args = node->args.args;
			for( int i = 0; i < args.size(); i++ )
			{
				auto& arg = args[i];
				visit_node(arg.get());

				if( i != args.size() - 1 )
				{
					append_span(NodeSpan{","});
					append_span(NodeSpan::Line());
				}
			}
		}
		append_span(NodeSpan::SoftLine());
	}

	append_span(NodeSpan{")"});
}

void
FormatParser::visit(ast::Statement const* node)
{
	visit_node(node->stmt.get());
}

void
FormatParser::visit(ast::Expression const* node)
{
	visit_node(node->base.get());
}

void
FormatParser::visit_node(ast::IAstNode const* node)
{
	node->visit(this);
	append_comments(node);
}

void
FormatParser::append_span(NodeSpan span)
{
	current_line.append_span(span);
}

void
FormatParser::append_comments(ast::IAstNode const* node)
{
	if( source && node->get_span().trailing_comments.size() != 0 )
	{
		for( auto& comment : node->get_span().trailing_comments )
		{
			auto token = source->at(comment);
			current_line.append_span(NodeSpan::LineSuffix(String(token.start, token.size)));
		}
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
{}

NodeSpan
FormatParser::get_span(Vec<Token> const* source, ast::IAstNode const* node)
{
	FormatParser g{source};

	node->visit(&g);

	return g.current_line.children[0];
}