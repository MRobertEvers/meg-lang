#pragma once
#include "../ast/IAstVisitor.h"
#include "SemaResult.h"

/**
 * @brief Performs semantic analysis on the AST produced by parser and produces HIR
 *
 * Since we are using a visit pattern that returns void,
 * there are a few conceptual helpers to make seem like we
 * can call visit and get a return value.
 *
 * First, each node is either a statement or an expression.
 * Nodes can return statements through the 'last_stmt' and
 * expressions through 'last_expr'. Just like the parser
 * however, the last result is return via a SemaResult<T>
 * and should be checked before continuing.
 */
class Sema : public IAstVisitor
{
public:
	virtual void visit(ast::Module const*) override;
	virtual void visit(ast::Function const*) override;
	virtual void visit(ast::Block const*) override;
	virtual void visit(ast::BinaryOperation const*) override;
	virtual void visit(ast::Number const*) override;
	virtual void visit(ast::Return const*) override;
	virtual void visit(ast::Prototype const*) override;
	virtual void visit(ast::TypeIdentifier const*) override;
	virtual void visit(ast::ValueIdentifier const*) override;
	virtual void visit(ast::Let const*) override;
	virtual void visit(ast::Struct const*) override;
	virtual void visit(ast::MemberReference const*) override;
	virtual void visit(ast::TypeDeclarator const*) override;
	virtual void visit(ast::If const*) override;
	virtual void visit(ast::Assign const*) override;
	virtual void visit(ast::While const*) override;
	virtual void visit(ast::Call const*) override;
	virtual void visit(ast::Statement const*) override;
}