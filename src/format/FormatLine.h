#pragma once

#include "ast/IAstNode.h"
#include "ast/IAstVisitor.h"
#include "common/String.h"
#include "common/Vec.h"
#include "lexer/token.h"

/**
 * Strategy for line formatting
 *
 * Break each node into a series of 'Spans'
 *
 * E.g. A Block node is internally.
 *
 * [
 * 	Span("{\n"),
 *  ...Span(stmts)
 * 	Span("}")
 * ]
 *
 * Notice Span("{\n") is in a single span. This means it is not allowed to be broken up.
 *
 * Gather into lines
 * [
 * 	[Span("{\n")],
 * 	[...OpaqueSpan(stmts), Span("}"), <implicit_newline>]
 * ]
 *
 * Note, if stmts produced more newline spans then it would look like this.
 *
 * [
 * 	[Span("{\n")],
 * 	[...OpaqueSpan(stmts[0:n]), Span("\n")]
 *  [...OpaqueSpan(stmts[n:end]), Span("}")]
 * ]
 *
 * Additionally, nodes corresponding to ast::nodes start as OpaqueSpans
 * OpaqueSpans keep track of their own length, it starts like this.
 *
 * [
 * 	[OpaqueSpan(block)]
 * ]
 *
 * Gather into lines
 * [
 * 	[Span("{\n")],
 * 	[OpaqueSpan(stmt1), OpaqueSpan(stmt2), ..., Span("}")]
 * ]
 *
 * Then recursively break lines as needed, image if stmt1 is a function call,
 * Each node can specify where line breaks are allowed. For a function, its
 * between params
 *
 * [
 * 	[Span("{\n")],
 * 	[Span("my_funccall(\n")],  	// From OpaqueSpan(stmt1)
 * 	[Span("<indent>arg1,\n")],	// From OpaqueSpan(stmt1)
 * 	[Span("<indent>arg2,\n")],	// From OpaqueSpan(stmt1)
 * 	[Span(")")],				// From OpaqueSpan(stmt1)
 *  [OpaqueSpan(stmt2), ..., Span("}")]
 * ]
 *
 * ## pseudo-code
 *
 * Start with a node.
 *
 * [
 * 	[OpaqueSpan(node)]
 * ]
 *
 * Expand that into lines by iterating over all
 * internal Spans recursively and see which ones end in newlines
 * (some nodes REQUIRE line breaks)
 *
 * [
 * 	[OpaqueSpan(n), ..., Span(..."\n")]
 *  [OpaqueSpan(n+1), ...]
 * 	...
 * ]
 *
 *
 * next_iteration = [] // empty line list
 * curr_iteration = lines
 * For each line, perform the following
 *
 * 	// Check if the line is too long
 * 	is_line_too_long = ...
 * 	if (!is_line_too_long)
 * 		next_iteration.push_back(line)
 * 		continue;
 *
 * 	// Get the highest priority line break in the list of spans,
 * 	// e.g. Break at function call arguments before '.' (MemberDereference)
 * 	int prio = get_highest_prio_break(line)
 *
 *	current_line = [] // list of spans
 * 	for each span in line
 * 		if span.prio >= prio
 *			broken_up_lines = break_up(span); // returns list of lines
 *
 *			current_line.push(broken_up_lines[0])
 *			next_iteration.push(current_line)
 *
 *			broken_up_lines.back().push(remaining spans in line)
 *
 *			next_iteration.push_all(broken_up_lines[1:])
 * 			break
 * 		else
 * 			current_line.push(span)
 *
 * Then perform the same operation on the output lines in next_iteration until
 * no lines are broken.
 *
 * Print the lines.
 *
 *
 */

class LineSpan
{
	enum class SpanType
	{
		span,
		opaque
	} type;

	String raw;
	ast::IAstNode const* opaque = nullptr;

	bool contains_terminal = false;

public:
	unsigned int priority = 0; // Max of all children.
	Vec<LineSpan> children;

	LineSpan(int prio, String raw)
		: priority(prio)
		, raw(raw)
		, type(SpanType::span){};
	LineSpan(int prio, ast::IAstNode const* opaque)
		: priority(prio)
		, opaque(opaque)
		, type(SpanType::opaque){};

	void append_span(LineSpan span);

	ast::IAstNode const* get_opaque() const { return opaque; }
	String get_raw() const { return raw; }

	bool is_raw() const { return type == SpanType::span; }
	bool is_terminator() const
	{
		if( type == SpanType::opaque )
		{
			return contains_terminal;
		}
		else
		{
			return raw.find("\n") != String::npos;
		}
	}

	int get_length() const
	{
		int len = 0;
		if( type == SpanType::opaque )
		{
			for( auto& child : children )
			{
				len += child.get_length();
			}
		}
		else
		{
			len += raw.length();

			if( is_terminator() )
			{
				len -= 1;
			}
		}

		return len;
	}
};

/**
 * @brief Gathers all the nodes that belong on a line.
 *
 *
 */
class GatherIntoLines : public IAstVisitor
{
	struct OpaqueScope
	{
		GatherIntoLines& ctx;
		LineSpan previous;
		bool has_previous = false;

		OpaqueScope(GatherIntoLines& ctx, ast::IAstNode const* node, int prio)
			: ctx(ctx)
			, previous(0, nullptr)
		{
			previous = ctx.current_line;

			ctx.current_line = LineSpan{prio, node};
		}
		OpaqueScope(GatherIntoLines& ctx, ast::IAstNode const* node)
			: OpaqueScope(ctx, node, 0){};

		~OpaqueScope()
		{
			previous.append_span(ctx.current_line);

			ctx.current_line = previous;
		}
	};

	struct IndentScope
	{
		GatherIntoLines& ctx;

		IndentScope(GatherIntoLines& ctx)
			: ctx(ctx)
		{
			ctx.current_indentation += 1;
		}

		~IndentScope() { ctx.current_indentation -= 1; }
	};

	LineSpan current_line{0, nullptr};

	int current_indentation = 0;
	bool need_indent = false;
	Vec<Token> const* source = nullptr;

	bool printed_source_newline = false;

public:
	GatherIntoLines(Vec<Token> const* source)
		: source(source){};

	GatherIntoLines(int current_indentation = 0)
		: current_indentation(current_indentation){};

	Vec<LineSpan> gather_into_lines(ast::IAstNode const* node);

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

	void append_span(LineSpan span);

	void append_newline_if_source(ast::IAstNode const* node, int threshold = 1);

	static LineSpan get_span(ast::IAstNode const* node, int indentation = 0);
};

class BreakNode : public IAstVisitor
{
	Vec<Vec<LineSpan>> result;

public:
	Vec<Vec<LineSpan>> break_node(ast::IAstNode const* node);

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