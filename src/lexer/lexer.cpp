#include "lexer.h"

#include "keywords.h"

#include <iomanip>
#include <iostream>

// clang-format off
#define CHAR_WHITESPACE_CASES \
    ' ': \ 
    case '\n': \
    case '\r': \
    case '\t'

#define CHAR_DIGIT_CASES \
         '0': \
    case '1': \
    case '2': \
    case '3': \
    case '4': \
    case '5': \
    case '6': \
    case '7': \
    case '8': \
    case '9'

#define CHAR_IDENTIFIER_START_CASES \
         'a': \
    case 'b': \
    case 'c': \
    case 'd': \
    case 'e': \
    case 'f': \
    case 'g': \
    case 'h': \
    case 'i': \
    case 'j': \
    case 'k': \
    case 'l': \
    case 'm': \
    case 'n': \
    case 'o': \
    case 'p': \
    case 'q': \
    case 'r': \
    case 's': \
    case 't': \
    case 'u': \
    case 'v': \
    case 'w': \
    case 'x': \
    case 'y': \
    case 'z': \
    case 'A': \
    case 'B': \
    case 'C': \
    case 'D': \
    case 'E': \
    case 'F': \
    case 'G': \
    case 'H': \
    case 'I': \
    case 'J': \
    case 'K': \
    case 'L': \
    case 'M': \
    case 'N': \
    case 'O': \
    case 'P': \
    case 'Q': \
    case 'R': \
    case 'S': \
    case 'T': \
    case 'U': \
    case 'V': \
    case 'W': \
    case 'X': \
    case 'Y': \
    case 'Z': \
    case '_'

#define CHAR_IDENTIFIER_CASES \
    CHAR_IDENTIFIER_START_CASES: \
    case CHAR_DIGIT_CASES

// clang-format on

void
Lexer::print_tokens(std::vector<Token> const& tokens)
{
	for( auto& tok : tokens )
	{
		std::string sz{tok.start, tok.start + tok.size};
		std::cout << std::setw(10) << sz << " : " << get_tokentype_string(tok) << std::endl;
	}
}

std::vector<Token>
Lexer::lex()
{
	std::vector<Token> result{};
	result.reserve(30);
	cursor_ = 0;

	for( ; cursor_ < input_len_; cursor_++ )
	{
		char c = input_[cursor_];

		switch( c )
		{
		case CHAR_WHITESPACE_CASES:

			break;

		case CHAR_IDENTIFIER_START_CASES:
			result.push_back(lex_consume_identifier());
			break;

		case CHAR_DIGIT_CASES:
			result.push_back(lex_consume_number());
			break;

		case '*':
		case '+':
		case '-':
		case '/':
		case '(':
		case ')':
		case '{':
		case '}':
		case ';':
		case ':':
			result.push_back(lex_consume_single());
			break;

		default:
			std::cout << "Unexpected character: '" << c << "'" << std::endl;
			break;
		}
	}

	return result;
}

Token
Lexer::lex_consume_number()
{
	Token token{};
	token.start = &input_[cursor_];
	token.literal_type = LiteralType::integer;
	token.type = TokenType::literal;
	for( ; cursor_ < input_len_; cursor_++ )
	{
		char c = input_[cursor_];

		switch( c )
		{
		case CHAR_DIGIT_CASES:
			token.size += 1;
			break;

		default:
			goto done;
			break;
		}
	}
done:
	cursor_--;
	return token;
}

Token
Lexer::lex_consume_single()
{
	Token token{};
	token.start = &input_[cursor_];
	token.size = 1;

	char c = input_[cursor_];

	switch( c )
	{
	case '*':
		token.type = TokenType::star;
		break;
	case '+':
		token.type = TokenType::plus;
		break;
	case '-':
		token.type = TokenType::minus;
		break;
	case '/':
		token.type = TokenType::slash;
		break;
	case '(':
		token.type = TokenType::open_paren;
		break;
	case ')':
		token.type = TokenType::close_paren;
		break;
	case '{':
		token.type = TokenType::open_curly;
		break;
	case '}':
		token.type = TokenType::close_curly;
		break;
	case ';':
		token.type = TokenType::semicolon;
		break;
	case ':':
		token.type = TokenType::colon;
		break;
	default:
		token.type = TokenType::bad;
		break;
	}

	return token;
}

Token
Lexer::lex_consume_identifier()
{
	Token token{};
	token.start = &input_[cursor_];
	for( ; cursor_ < input_len_; cursor_++ )
	{
		char c = input_[cursor_];

		switch( c )
		{
		case CHAR_IDENTIFIER_CASES:
			token.size += 1;
			break;

		default:
			goto done;
			break;
		}
	}
done:
	token.type = get_identifier_or_keyword_type(token);

	cursor_--;

	return token;
}
