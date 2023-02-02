
#include "Sema2.h"

#include <iostream>

using namespace ast;

void
Sema2::sema(ast::AstNode const& node)
{
	ast::Span span = std::visit(
		[](auto&& arg) -> auto { return arg.get_span(); }, node);

	std::cout << span.size << std::endl;
}