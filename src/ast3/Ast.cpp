#include "Ast.h"

Ast::Ast(){};

AstNode*
Ast::root()
{
	return &nodes.front();
}