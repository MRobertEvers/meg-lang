#pragma once
#include "NodeSpan.h"
#include "ast/IAstNode.h"
#include "ast/IAstVisitor.h"
#include "common/String.h"
#include "common/Vec.h"
#include "lexer/token.h"

/**
 * @brief Create the "Formatting AST".
 *
 */
class FormatParser : public IAstVisitor
{
	struct GroupScope
	{
		FormatParser& ctx;
		NodeSpan previous;
		GroupScope(FormatParser& ctx);
		~GroupScope();
	};

	struct DocumentScope
	{
		FormatParser& ctx;
		NodeSpan previous;
		DocumentScope(FormatParser& ctx);
		~DocumentScope();
	};

	struct IndentScope
	{
		FormatParser& ctx;
		NodeSpan previous;
		IndentScope(FormatParser& ctx);
		~IndentScope();
	};

	NodeSpan current_line{};

	Vec<Token> const* source = nullptr;

public:
	FormatParser(Vec<Token> const* source)
		: source(source){};

	// Vec<NodeSpan> gather_into_lines(ast::IAstNode const* node);

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

private:
	void visit_node(ast::IAstNode const* node);
	void append_span(NodeSpan span);
	void append_comments(ast::IAstNode const* node);
	void append_newline_if_source(ast::IAstNode const* node, int threshold = 1);

public:
	static NodeSpan get_span(Vec<Token> const* source, ast::IAstNode const* node);
};
