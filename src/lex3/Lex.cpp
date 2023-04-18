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
	case TokenKind::Pipe:
	case TokenKind::Ampersand:
	case TokenKind::Minus:
	case TokenKind::Eq:
	case TokenKind::Lt:
	case TokenKind::Gt:
	case TokenKind::Exclam:
	case TokenKind::Colon:
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
		break;
	case '&':
		tok_kind = TokenKind::Ampersand;
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
	case '^':
		tok_kind = TokenKind::Caret;
		cursor_ += 1;
		break;
	case '~':
		tok_kind = TokenKind::Tilde;
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
	case '|':
		tok_kind = TokenKind::Pipe;
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
	case TokenKind::Ampersand:
		if( match(view, "&&") )
			kind = TokenKind::AmpAmp;
		break;
	case TokenKind::Pipe:
		if( match(view, "||") )
			kind = TokenKind::PipePipe;
		break;
	case TokenKind::Colon:
		if( match(view, "::") )
			kind = TokenKind::ColonColon;
		break;
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

	if( view.size == 0 )
	{
		view.size += 1;
		cursor_ += 1;
	}

	return Token(view, kind);
}

bool
Lex::match(TokenView& tok_view, char const* matcher)
{
	int start_size = tok_view.size;

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
	tok_view.size = start_size;
	cursor_ = start;
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