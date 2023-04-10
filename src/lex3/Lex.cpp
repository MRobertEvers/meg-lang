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
		token = tok_number_literal();
		break;
	case TokenKind::Minus:
	case TokenKind::Eq:
	case TokenKind::Lt:
	case TokenKind::Gt:
	case TokenKind::Exclam:
		token = tok_ambiguous(tok_kind);
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
Lex::is_done() const
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
	case '[':
		tok_kind = TokenKind::OpenSquare;
		cursor_ += 1;
		break;
	case ']':
		tok_kind = TokenKind::CloseSquare;
		cursor_ += 1;
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
	case '&':
		tok_kind = TokenKind::Ampersand;
		cursor_ += 1;
		break;
	case '.':
		tok_kind = TokenKind::Dot;
		cursor_ += 1;
		break;
	case '*':
		tok_kind = TokenKind::Star;
		cursor_ += 1;
		break;
	case '/':
		tok_kind = TokenKind::Slash;
		cursor_ += 1;
		break;
	case '+':
		tok_kind = TokenKind::Plus;
		cursor_ += 1;
		break;
	case '-':
		tok_kind = TokenKind::Minus;
		break;
	case ',':
		tok_kind = TokenKind::Comma;
		cursor_ += 1;
		break;
	case '=':
		tok_kind = TokenKind::Eq;
		break;
	case '<':
		tok_kind = TokenKind::Lt;
		break;
	case '>':
		tok_kind = TokenKind::Gt;
		break;
	case '!':
		tok_kind = TokenKind::Exclam;
		break;
	default:
		cursor_ += 1;
		break;
	}

	return tok_kind;
}

Token
Lex::tok_ambiguous(TokenKind kind)
{
	TokenView view(head());

	switch( kind )
	{
	case TokenKind::Minus:
		if( match(view, "->") )
			kind = TokenKind::SkinnyArrow;
		break;
	case TokenKind::Eq:
		if( match(view, "==") )
			kind = TokenKind::EqEq;
		else if( match(view, "=>") )
			kind = TokenKind::FatArrow;
		break;
	case TokenKind::Lt:
		if( match(view, "<=") )
			kind = TokenKind::LtEq;
		break;
	case TokenKind::Gt:
		if( match(view, ">=") )
			kind = TokenKind::GtEq;
		break;
	case TokenKind::Exclam:
		if( match(view, "!=") )
			kind = TokenKind::ExclamEq;
		break;
	default:
		break;
	}

	return Token(view, kind);
}

bool
Lex::match(TokenView& tok_view, char const* matcher)
{
	int start = cursor_;
	int match_len = strlen(matcher);
	int i;
	for( i = 0; i < match_len && !is_done(); i++ )
	{
		char test = matcher[i];

		if( test != current() )
			goto reset;

		cursor_ += 1;
		tok_view.size += 1;
	}

	if( match_len != i )
		goto reset;

	return true;

reset:
	tok_view.size = 1;
	cursor_ = start + 1;
	return false;
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

	return Token(tok_view, tok_kind);
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