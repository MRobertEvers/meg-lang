#include "NodeFormatter.h"

#include "FormatParser.h"
#include "ast/ast.h"

Vec<Vec<NodeSpan>>
NodeFormatter::break_node(ast::IAstNode const* node)
{
	result.clear();
	node->visit(this);

	if( result.size() == 0 )
	{
		Vec<NodeSpan> dummy;
		auto span = FormatParser::get_span(source, node);
		span.priority = 0;
		dummy.push_back(span);
		result.push_back(dummy);
	}

	return result;
}

void
NodeFormatter::visit(ast::Module const* node)
{}

void
NodeFormatter::visit(ast::Function const* node)
{}

void
NodeFormatter::visit(ast::Block const* node)
{}

void
NodeFormatter::visit(ast::BinaryOperation const* node)
{}

void
NodeFormatter::visit(ast::Number const* node)
{}

void
NodeFormatter::visit(ast::Return const* node)
{}

void
NodeFormatter::visit(ast::Prototype const* node)
{
	Vec<NodeSpan> line;

	line.push_back(NodeSpan{0, "fn "});
	line.push_back(FormatParser::get_span(source, node->Name.get()));
	line.push_back(NodeSpan{0, "(\n"});

	result.push_back(line);
	line.clear();

	auto& args = node->get_parameters()->Parameters;

	for( int i = 0; i < args.size(); i++ )
	{
		auto& arg = args[i];
		line.push_back(FormatParser::get_span(source, arg->Name.get(), 1));

		line.push_back(NodeSpan{0, ": "});

		line.push_back(FormatParser::get_span(source, arg->Type.get(), 0));

		if( i != args.size() - 1 )
		{
			line.push_back(NodeSpan{0, ",\n"});
		}
		else
		{
			line.push_back(NodeSpan{0, "\n"});
		}
		result.push_back(line);
		line.clear();
	}

	line.push_back(NodeSpan{0, "): "});

	line.push_back(FormatParser::get_span(source, node->ReturnType.get()));

	line.push_back(NodeSpan{0, " "});
	result.push_back(line);
}

void
NodeFormatter::visit(ast::ValueIdentifier const* node)
{}

void
NodeFormatter::visit(ast::TypeIdentifier const* node)
{}

void
NodeFormatter::visit(ast::Let const* node)
{}

void
NodeFormatter::visit(ast::Struct const* node)
{}

void
NodeFormatter::visit(ast::MemberReference const* node)
{}

void
NodeFormatter::visit(ast::TypeDeclarator const* node)
{}

void
NodeFormatter::visit(ast::If const* node)
{}

void
NodeFormatter::visit(ast::Assign const* node)
{}

void
NodeFormatter::visit(ast::While const* node)
{}

void
NodeFormatter::visit(ast::Call const* node)
{}

void
NodeFormatter::visit(ast::Statement const* node)
{}

void
NodeFormatter::visit(ast::Expression const* node)
{}