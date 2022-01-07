#pragma once

namespace ast
{
class Module;
class Function;
class Block;
class BinaryOperation;
class Number;
class Return;
class Prototype;
class TypeIdentifier;
class ValueIdentifier;
class Let;
class Struct;
} // namespace ast

class IAstVisitor
{
public:
	virtual void visit(ast::Module const*) = 0;
	virtual void visit(ast::Function const*) = 0;
	virtual void visit(ast::Block const*) = 0;
	virtual void visit(ast::BinaryOperation const*) = 0;
	virtual void visit(ast::Number const*) = 0;
	virtual void visit(ast::Return const*) = 0;
	virtual void visit(ast::Prototype const*) = 0;
	virtual void visit(ast::TypeIdentifier const*) = 0;
	virtual void visit(ast::ValueIdentifier const*) = 0;
	virtual void visit(ast::Let const*) = 0;
	virtual void visit(ast::Struct const*) = 0;
};
