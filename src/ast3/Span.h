

#pragma once

#include <vector>

/**
 * @brief Used to keep track of meta-information in the AST.
 *
 * Tracks information for elements of the input source that are valid syntax, but otherwise
 * not represented in the AST. E.g. Comments
 *
 */
struct Span
{
	// The first token used to produce the AST node.
	int start = 0;
	int size = 0;

	// Leading comments are orphaned on their own line,
	// like this comment.
	std::vector<int> leading_comments;

	// Trailing comments are comments that are not on their own line.
	// E.g. if (my_bool) // trailing comment
	std::vector<int> trailing_comments;

	Span() = default;
	Span(int start, int size)
		: start(start)
		, size(size){};
};