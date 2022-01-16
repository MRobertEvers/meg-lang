#pragma once
#include "FormatLine.h"
#include "ast/IAstVisitor.h"
#include "lexer/token.h"

/**
 * @brief Performs formatting on AST
 *
 * Internally, this breaks the AST into a separate Span representation
 * then outputs the lines.
 *
 */
class Format : public IAstVisitor
{
	Vec<Token> const& source;

	Vec<LineSpan> lines;

public:
	Format(Vec<Token> const& source);
	~Format();
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