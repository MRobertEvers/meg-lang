#pragma once
#include "NodeSpan.h"
#include "ast/IAstNode.h"
#include "ast/IAstVisitor.h"
#include "common/String.h"
#include "common/Vec.h"
#include "lexer/token.h"

class FormatParser : public IAstVisitor
{
	struct OpaqueScope
	{
		FormatParser& ctx;
		NodeSpan previous;
		bool has_previous = false;

		OpaqueScope(FormatParser& ctx, ast::IAstNode const* node, int prio)
			: ctx(ctx)
			, previous(0, nullptr)
		{
			previous = ctx.current_line;

			ctx.current_line = NodeSpan{prio, node};
		}
		OpaqueScope(FormatParser& ctx, ast::IAstNode const* node)
			: OpaqueScope(ctx, node, 0){};

		~OpaqueScope()
		{
			previous.append_span(ctx.current_line);

			ctx.current_line = previous;
		}
	};

	struct IndentScope
	{
		FormatParser& ctx;

		IndentScope(FormatParser& ctx)
			: ctx(ctx)
		{
			ctx.current_indentation += 1;
		}

		~IndentScope() { ctx.current_indentation -= 1; }
	};

	NodeSpan current_line{0, nullptr};

	int current_indentation = 0;
	bool need_indent = false;
	bool printed_source_newline = false;
	Vec<Token> const* source = nullptr;

public:
	FormatParser(Vec<Token> const* source)
		: source(source){};

	FormatParser(int current_indentation = 0)
		: current_indentation(current_indentation){};

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

	void append_span(NodeSpan span);

	void append_newline_if_source(ast::IAstNode const* node, int threshold = 1);

	static NodeSpan
	get_span(Vec<Token> const* source, ast::IAstNode const* node, int indentation = 0);
};
