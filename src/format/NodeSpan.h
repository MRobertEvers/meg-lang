#pragma once

#include "common/String.h"
#include "common/Vec.h"

class NodeSpan
{
	bool contains_break = false;

public:
	enum class SpanType
	{
		text,
		document,  // A list of concatenated nodes.
		line,	   // Line break or space if group fits on line.
		soft_line, // Line break or space if fits on line
		hard_line, // Always line break,
		indent,
		dedent,
		line_suffix, // Use for line-end comments
		group
	} type;

public:
	// used in TEXT node only
	String content;
	Vec<NodeSpan> children;
	NodeSpan()
		: type(SpanType::text)
		, content(""){};
	NodeSpan(char const* content)
		: type(SpanType::text)
		, content(content){};
	NodeSpan(String content)
		: type(SpanType::text)
		, content(content){};

	NodeSpan(SpanType type)
		: type(type){};

	NodeSpan(SpanType type, String content)
		: type(type)
		, content(content){};

	void append_span(NodeSpan span);

	bool is_break() const;
	int get_length() const;

public:
	static NodeSpan Text(String text);
	static NodeSpan Line();
	static NodeSpan SoftLine();
	static NodeSpan HardLine();
	static NodeSpan Document();
	static NodeSpan Indent();
	static NodeSpan Dedent();
	static NodeSpan LineSuffix(String text);
	static NodeSpan Group();
};
