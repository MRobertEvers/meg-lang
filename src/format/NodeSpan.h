#pragma once
#include "ast/IAstNode.h"
#include "common/String.h"
#include "common/Vec.h"

class NodeSpan
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
	Vec<NodeSpan> children;

	NodeSpan(int prio, String raw)
		: priority(prio)
		, raw(raw)
		, type(SpanType::span){};
	NodeSpan(int prio, ast::IAstNode const* opaque)
		: priority(prio)
		, opaque(opaque)
		, type(SpanType::opaque){};

	void append_span(NodeSpan span);

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
