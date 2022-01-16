#pragma once
#include "NodeSpan.h"
#include "ast/IAstNode.h"
#include "ast/IAstVisitor.h"
#include "common/String.h"
#include "common/Vec.h"
#include "lexer/token.h"

class NodeFormatter : public IAstVisitor
{
	Vec<Vec<NodeSpan>> result;
	Vec<Token> const* source = nullptr;

public:
	NodeFormatter(Vec<Token> const* source)
		: source(source){};

	Vec<Vec<NodeSpan>> break_node(ast::IAstNode const* node);

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