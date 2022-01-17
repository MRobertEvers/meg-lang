#include "ParseTrail.h"

Span
ParseTrail::mark()
{
	size = cursor.get_index() - start;

	auto span = Span{start, size};

	// TODO: Flag on this.
	// Check for comments.
	for( int i = 0; i < size; ++i )
	{
		auto ind = start + i;
		auto passed_tok = cursor.peek(ind);
		if( passed_tok.type == TokenType::line_comment )
		{
			auto already_captured = meta.comments.find(ind);
			if( already_captured == meta.comments.end() )
			{
				span.trailing_comments.push_back(ind);
				meta.comments.insert(ind);
			}
		}
	}

	return span;
}