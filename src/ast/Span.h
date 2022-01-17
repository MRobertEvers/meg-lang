

#pragma once

#include "common/Vec.h"

namespace ast
{

/**
 * @brief Used to keep track of the appears of a node in the source code for pretty printing.
 *
 * Tracks information for elements of the input source that are valid syntax, but otherwise
 * not represented in the AST. E.g. Comments
 *
 */
struct Span
{
	int start = 0;
	int size = 0;

	Vec<int> trailing_comments;

	Span(){};
	Span(int start, int size)
		: start(start)
		, size(size){};
};
} // namespace ast