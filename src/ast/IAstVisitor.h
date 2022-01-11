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
class TypeDeclarator;
class ValueIdentifier;
class Let;
class Struct;
class If;
class MemberReference;
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
	virtual void visit(ast::MemberReference const*) = 0;
	virtual void visit(ast::TypeDeclarator const*) = 0;
	virtual void visit(ast::If const*) = 0;
};
