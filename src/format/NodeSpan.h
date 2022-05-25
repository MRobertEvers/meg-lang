#pragma once

#include "common/String.h"
#include "common/Vec.h"

/**
 * @brief Represents a single node in the "Formatting AST".
 *
 */
class NodeSpan
{
	bool contains_break = false;

public:
	enum class SpanType
	{
		text,
		document,  // A list of concatenated nodes.
		line,	   // Line break or space if group fits on line.
		soft_line, // Line break or nothing if fits on line
		hard_line, // Always line break,
		indent,
		min_padding, // Add minimum spacing between non-newline/whitespace elements
		min_newline, // Similar to min_padding.
		line_suffix, // Use for line-end comments
		group
	} type;

public:
	// used in TEXT node only
	String content;

	// used in MIN_PADDING_NODE only
	int min_spacing = 0;

	Vec<NodeSpan> children;

	// TODO: Use asserts to verify nodes are constructed correctly.
	// TEXT Node helpers
	NodeSpan()
		: type(SpanType::text)
		, content(""){};
	NodeSpan(char const* content)
		: type(SpanType::text)
		, content(content){};
	NodeSpan(String content)
		: type(SpanType::text)
		, content(content){};

	NodeSpan(SpanType type, int num)
		: type(type)
		, min_spacing(num)
	{
		assert(type == SpanType::min_padding || type == SpanType::min_newline);
	};

	NodeSpan(SpanType type)
		: type(type){};

	NodeSpan(SpanType type, String content)
		: type(type)
		, content(content){};

	void append_span(NodeSpan span);

	bool is_break() const;

public:
	static NodeSpan Text(String text);
	static NodeSpan Line();
	static NodeSpan SoftLine();
	static NodeSpan HardLine();
	static NodeSpan Document();
	static NodeSpan Indent();
	static NodeSpan MinSpacing(int spacing);
	static NodeSpan MinNewLine(int spacing);
	static NodeSpan LineSuffix(String text);
	static NodeSpan Group();
};
