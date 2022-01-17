#include "NodeSpan.h"

void
NodeSpan::append_span(NodeSpan span)
{
	if( span.is_break() )
	{
		contains_break = true;
	}

	children.push_back(span);
}

bool
NodeSpan::is_break() const
{
	if( type == SpanType::hard_line )
	{
		return true;
	}
	else
	{
		return contains_break;
	}
}

int
NodeSpan::get_length() const
{
	int len = 0;

	return len;
}

NodeSpan
NodeSpan::Text(String text)
{
	return NodeSpan{SpanType::text, text};
}

NodeSpan
NodeSpan::Line()
{
	return SpanType::line;
}

NodeSpan
NodeSpan::SoftLine()
{
	return SpanType::soft_line;
}

NodeSpan
NodeSpan::HardLine()
{
	return SpanType::hard_line;
}

NodeSpan
NodeSpan::Document()
{
	return SpanType::document;
}

NodeSpan
NodeSpan::Indent()
{
	return SpanType::indent;
}

NodeSpan
NodeSpan::Dedent()
{
	return SpanType::dedent;
}

NodeSpan
NodeSpan::LineSuffix(String text)
{
	NodeSpan line_suffix = SpanType::line_suffix;
	line_suffix.append_span(text);
	return line_suffix;
}

NodeSpan
NodeSpan::Group()
{
	return SpanType::group;
}
