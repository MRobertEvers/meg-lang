#include "Lex.h"

#include "character_sets.h"
#include "keywords.h"

#include <cstring>

Lex::Lex(char const* input)
	: input_(input)
	, cursor_(0)
	, input_len_(strlen(input))
	, curr_line_(0)
{
	auto dummy = Token();
	consume_whitespace(dummy);
}

Token
Lex::next()
{
	// Some tokens have different start characters and are variable length.
	// If that is the case this switch will handle the remaining chars.
	TokenKind tok_kind = tok_start();
	TokenView tok_view(head());
	Token token;

	switch( tok_kind )
	{
	case TokenKind::Identifier:
		token = tok_identifier();
		break;
	case TokenKind::NumberLiteral:
		token = tok_identifier();
		break;
	default:
		token = Token(tok_view, tok_kind);
		break;
	}

	consume_whitespace(token);

	return token;
}

char
Lex::current()
{
	return input_[cursor_];
}

char const*
Lex::head()
{
	return input_ + cursor_;
}

bool
Lex::is_done()
{
	return cursor_ >= input_len_;
}

TokenKind
Lex::tok_start()
{
	TokenKind tok_kind = TokenKind::Bad;
	if( is_done() )
		return TokenKind::Eof;

	char c = current();
	switch( c )
	{
	case CHAR_DIGIT_CASES:
		tok_kind = TokenKind::NumberLiteral;
		break;
	case CHAR_IDENTIFIER_START_CASES:
		tok_kind = TokenKind::Identifier;
		break;
	case '(':
		tok_kind = TokenKind::OpenParen;
		cursor_ += 1;
		break;
	case ')':
		tok_kind = TokenKind::CloseParen;
		cursor_ += 1;
		break;
	case '{':
		tok_kind = TokenKind::OpenCurly;
		cursor_ += 1;
		break;
	case '}':
		tok_kind = TokenKind::CloseCurly;
		cursor_ += 1;
		break;
	case ';':
		tok_kind = TokenKind::SemiColon;
		cursor_ += 1;
		break;
	case ':':
		tok_kind = TokenKind::Colon;
		cursor_ += 1;
		break;
	default:
		cursor_ += 1;
		break;
	}

	return tok_kind;
}

Token
Lex::tok_identifier()
{
	TokenView tok_view(head());

	while( !is_done() )
	{
		char c = current();

		switch( c )
		{
		case CHAR_IDENTIFIER_CASES:
			tok_view.size += 1;
			break;

		default:
			goto done;
			break;
		}

		cursor_++;
	}

done:
	TokenKind tok_kind = get_identifier_or_keyword_type(tok_view);

	return Token(tok_view, TokenKind::Identifier);
}

Token
Lex::tok_number_literal()
{
	TokenView tok_view(head());

	while( !is_done() )
	{
		char c = current();

		switch( c )
		{
		case CHAR_DIGIT_CASES:
			tok_view.size += 1;
			break;

		default:
			goto done;
			break;
		}

		cursor_++;
	}

done:
	return Token(tok_view, TokenKind::NumberLiteral);
}

void
Lex::consume_whitespace(Token& tok)
{
	while( !is_done() )
	{
		char c = current();

		switch( c )
		{
		case CHAR_WHITESPACE_CASES:
			// TODO: Track new lines.
			break;
		default:
			goto done;
			break;
		}

		cursor_++;
	}
done:
	return;
}