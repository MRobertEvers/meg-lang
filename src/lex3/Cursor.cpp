#include "Cursor.h"

Cursor::Cursor(char const* input)
	: lex(input)
{}

bool
Cursor::at_end() const
{
	return this->lex.is_done();
}

Token
Cursor::token_at(int ind)
{
	return this->tokens.at(ind);
}

Token
Cursor::peek()
{
	return *current();
}

ConsumeResult
Cursor::consume_if_expected(TokenKind expected)
{
	Token* cur = current();
	if( cur->kind == expected )
		return consume(expected);

	return ConsumeResult::Fail(consume(false));
}

ConsumeResult
Cursor::consume(TokenKind expected)
{
	return consume({expected});
}

ConsumeResult
Cursor::consume(std::initializer_list<TokenKind> expecteds)
{
	Token* cur = current();
	for( TokenKind kind : expecteds )
	{
		if( cur->kind == kind )
			return ConsumeResult(consume(true));
	}

	return ConsumeResult::Fail(consume(false));
}

Token*
Cursor::current()
{
	if( queue.size() != 0 )
		return queue.front();

	Token lex_next = lex.next();
	while( lex_next.kind == TokenKind::LineComment )
		lex_next = lex.next();

	tokens.push_back(lex_next);

	Token* next = &tokens.back();
	queue.push_back(next);

	return next;
}

Token*
Cursor::consume(bool consume)
{
	Token* cur = current();
	if( consume )
		queue.pop_front();

	return cur;
}