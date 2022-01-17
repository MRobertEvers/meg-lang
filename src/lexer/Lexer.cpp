#include "Lexer.h"

#include "character_sets.h"
#include "keywords.h"

#include <iomanip>
#include <iostream>

LexResult::LexResult(unsigned int num_lines, Vec<char const*> lines, Vec<Token> tokens)
	: tokens(std::move(tokens))
{
	markers.lines = lines;
	markers.num_lines = num_lines;

	for( auto& tok : this->tokens )
	{
		tok.neighborhood.lines = markers;
	}
}

void
Lexer::print_tokens(Vec<Token> const& tokens)
{
	for( auto& tok : tokens )
	{
		std::string sz{tok.start, tok.start + tok.size};
		std::cout << std::setw(10) << sz << " : " << get_tokentype_string(tok) << std::endl;
	}
}

LexResult
Lexer::lex()
{
	Vec<Token> tokens{};
	tokens.reserve(30);
	cursor_ = 0;
	lines_.push_back(&input_[cursor_]);

	for( ; cursor_ < input_len_; cursor_++ )
	{
		char c = input_[cursor_];

		switch( c )
		{
		case CHAR_WHITESPACE_CASES:
		{
			if( c == '\n' )
			{
				lines_.push_back(&input_[cursor_]);
				curr_line_++;
				track_newline(tokens);
			}
		}
		break;
		case CHAR_IDENTIFIER_START_CASES:
			tokens.push_back(lex_consume_identifier());
			break;

		case CHAR_DIGIT_CASES:
			tokens.push_back(lex_consume_number());
			break;

		case '/':
			// Could be '/' (division) or '//' (a comment)
			if( peek("/") )
			{
				tokens.push_back(lex_consume_line_comment());
				break;
			}
			else
			{
				// Fall through
			}
		case '*':
		case '+':
		case '-':
		case '(':
		case ')':
		case '{':
		case '}':
		case ';':
		case ':':
		case ',':
		case '.':
		case '>':
		case '=':
			tokens.push_back(lex_consume_single());
			break;

		default:
			std::cout << "Unexpected character: '" << c << "'" << std::endl;
			break;
		}
	}

	tokens.emplace_back(TokenType::eof);

	auto result = LexResult{curr_line_, lines_, tokens};

	return result;
}

Token
Lexer::lex_consume_number()
{
	Token token{};
	token.start = &input_[cursor_];
	token.literal_type = LiteralType::integer;
	token.type = TokenType::literal;
	token.neighborhood.line_num = curr_line_;

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
	token.neighborhood.line_num = curr_line_;

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
	case ',':
		token.type = TokenType::comma;
		break;
	case '=':
		token.type = TokenType::equal;
		break;
	case '.':
		token.type = TokenType::dot;
		break;
	case '>':
		token.type = TokenType::gt;
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
	token.neighborhood.line_num = curr_line_;

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

Token
Lexer::lex_consume_line_comment()
{
	Token token{};
	token.type = TokenType::line_comment;
	token.start = &input_[cursor_];
	token.size = 1;
	token.neighborhood.line_num = curr_line_;

	char c = input_[++cursor_];
	while( c != '\n' )
	{
		token.size += 1;
		c = input_[++cursor_];
	}

	--cursor_;

	return token;
}

void
Lexer::track_newline(std::vector<Token>& tokens)
{
	if( tokens.size() != 0 )
	{
		tokens.back().num_trailing_newlines += 1;
	}
}

bool
Lexer::peek(char const* seq)
{
	auto len = strlen(seq);

	for( int i = 0; i < len; ++i )
	{
		int look_ahead = cursor_ + i + 1;
		if( look_ahead < input_len_ )
		{
			auto c = input_[look_ahead];
			if( c != seq[i] )
			{
				return true;
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}