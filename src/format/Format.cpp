#include "Format.h"

#include "../ast/ast.h"
#include "FormatLine.h"

#include <iomanip>
#include <iostream>

Format::Format(Vec<Token> const& source)
	: source(source)
{}

Format::~Format()
{}

void
Format::visit(ast::Module const* node)
{
	GatherIntoLines g{&source};
	auto lines = g.gather_into_lines(node);

	std::cout << lines.size() << std::endl;
}

void
Format::visit(ast::Function const* node)
{
	node->Proto->visit(this);

	node->Body->visit(this);
}

void
Format::visit(ast::Block const* node)
{
	std::cout << "{" << std::endl;

	for( auto& stmt : node->statements )
	{
		stmt->visit(this);
	}

	std::cout << "}" << std::endl;
}

void
Format::visit(ast::BinaryOperation const* node)
{
	node->LHS->visit(this);
	std::cout << node->Op;
	node->RHS->visit(this);
}

void
Format::visit(ast::Number const* node)
{
	std::cout << node->Val;
}

void
Format::visit(ast::Return const* node)
{
	std::cout << "return ";
	node->ReturnExpr->visit(this);
	std::cout << ";" << std::endl;
}

void
Format::visit(ast::Prototype const* node)
{
	std::cout << "fn ";
	node->Name->visit(this);
	std::cout << "(";

	auto& args = node->get_parameters()->Parameters;

	for( int i = 0; i < args.size(); i++ )
	{
		auto& arg = args[i];

		arg->Name->visit(this);

		std::cout << ": ";

		arg->Type->visit(this);

		if( i != args.size() - 1 )
		{
			std::cout << ", ";
		}
	}
	std::cout << "): ";

	node->ReturnType->visit(this);
}

void
Format::visit(ast::ValueIdentifier const* node)
{
	std::cout << node->get_name();
}

void
Format::visit(ast::TypeIdentifier const* node)
{
	std::cout << node->get_name();
}

void
Format::visit(ast::Let const* node)
{
	std::cout << "let ";
	node->Name->visit(this);
	if( !node->Type->is_empty() )
	{
		std::cout << ": ";
		node->Type->visit(this);
	}
	std::cout << " = ";
	node->RHS->visit(this);
	std::cout << ";" << std::endl;
}

void
Format::visit(ast::Struct const* node)
{
	std::cout << "struct ";
	node->TypeName->visit(this);
	std::cout << " {" << std::endl;
	for( auto& m : node->MemberVariables )
	{
		m->Name->visit(this);

		std::cout << ": ";

		m->Type->visit(this);

		std::cout << ";" << std::endl;
	}
	std::cout << "}" << std::endl;
}

void
Format::visit(ast::MemberReference const* node)
{
	node->base->visit(this);
	std::cout << ".";
	node->name->visit(this);
}

void
Format::visit(ast::TypeDeclarator const* node)
{
	auto base = node->get_base();
	if( base )
	{
		base->visit(this);
	}
	std::cout << node->get_name();
}

void
Format::visit(ast::If const* node)
{
	std::cout << "if ";
	node->condition->visit(this);
	node->then_block->visit(this);

	if( !node->else_block.is_null() )
	{
		std::cout << "else ";
		node->else_block->visit(this);
	}
}

void
Format::visit(ast::Assign const* node)
{
	node->lhs->visit(this);
	std::cout << " = ";
	node->rhs->visit(this);
	std::cout << ";" << std::endl;
}

void
Format::visit(ast::While const* node)
{
	std::cout << "while ";
	node->condition->visit(this);
	node->loop_block->visit(this);
}

void
Format::visit(ast::Call const* node)
{
	node->call_target->visit(this);
	std::cout << "(";

	auto& args = node->args.args;
	for( int i = 0; i < args.size(); i++ )
	{
		auto& arg = args[i];
		arg->visit(this);

		if( i != args.size() - 1 )
		{
			std::cout << ", ";
		}
	}

	std::cout << ")";
}

void
Format::visit(ast::Statement const* node)
{
	node->stmt->visit(this);
}

void
Format::visit(ast::Expression const* node)
{
	node->base->visit(this);
}