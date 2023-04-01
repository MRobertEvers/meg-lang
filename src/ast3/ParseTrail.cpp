#include "ParseTrail.h"

using namespace ast;

Span
ParseTrail::mark()
{
	size = cursor.get_index() - start;

	auto span = Span{start, size};

	bool leading_mode = true;

	// TODO: Flag on this.
	// Check for comments.
	for( int i = 0; i < size; ++i )
	{
		auto ind = start + i;
		auto passed_tok = cursor.peek(ind);
		if( passed_tok.type == TokenKind::LineComment )
		{
			auto already_captured = meta.comments.find(ind);
			if( already_captured == meta.comments.end() )
			{
				if( leading_mode )
				{
					span.leading_comments.push_back(ind);
					meta.comments.insert(ind);
				}
				else
				{
					span.trailing_comments.push_back(ind);
					meta.comments.insert(ind);
				}
			}
		}
		else
		{
			leading_mode = false;
		}
	}

	return span;
}