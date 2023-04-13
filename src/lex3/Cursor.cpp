#include "Cursor.h"

Cursor::Cursor(char const* input)
	: lex(input)
{}

int
Cursor::save_point() const
{
	return ind;
}

void
Cursor::reset_point(int reset)
{
	ind = reset;
}

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
	if( ind != tokens.size() )
		return &tokens.at(ind);

	Token lex_next = lex.next();
	while( lex_next.kind == TokenKind::LineComment )
		lex_next = lex.next();

	tokens.push_back(lex_next);

	Token* next = &tokens.back();

	return next;
}

Token*
Cursor::consume(bool consume)
{
	Token* cur = current();

	if( consume )
		ind += 1;

	return cur;
}