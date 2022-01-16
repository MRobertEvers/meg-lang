#include "NodeSpan.h"

void
NodeSpan::append_span(NodeSpan span)
{
	if( span.priority > priority )
	{
		priority = span.priority;
	}

	if( span.is_terminator() )
	{
		contains_terminal = true;
	}

	children.push_back(span);
}