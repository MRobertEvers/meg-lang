#include "Format.h"

#include "../ast/ast.h"

#include <iomanip>
#include <iostream>

void
Format::visit(ast::Module const* node)
{
	for( auto const& s : node->statements )
	{
		s->visit(this);
	}
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
	node->Name.visit(this);
	std::cout << "(";

	auto& args = node->get_args();
	for( int i = 0; i < args.size(); i++ )
	{
		auto& arg = args[i];

		arg->first.visit(this);

		std::cout << ": ";

		arg->second.visit(this);

		if( i != args.size() - 1 )
		{
			std::cout << ", ";
		}
	}
	std::cout << ")" << std::endl;
}

void
Format::visit(ast::Identifier const* node)
{
	std::cout << node->name;
}

void
Format::visit(ast::Let const* node)
{
	std::cout << "let ";
	node->identifier.visit(this);
	if( !node->type.is_empty )
	{
		std::cout << ": ";
		node->type.visit(this);
	}
	std::cout << " = ";
	node->rhs->visit(this);
	std::cout << ";" << std::endl;
}