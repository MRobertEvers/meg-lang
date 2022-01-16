#pragma once

#include "ast/IAstNode.h"
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
void pretty_print_ast(Vec<Token> const& source, ast::IAstNode const* node);