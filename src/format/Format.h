#pragma once
#include "../ast/IAstVisitor.h"
#include "lexer/token.h"

class Format : public IAstVisitor
{
	Vec<Token> const& source;

public:
	Format(Vec<Token> const& source);

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
	virtual void visit(ast::Expression const*) override;
};