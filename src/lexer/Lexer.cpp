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

		case '(':
		case ')':
		case '{':
		case '}':
		case ';':
		case ':':
		case ',':
			tokens.push_back(lex_consume_single());
			break;
		case '.':
		case '/':
		case '>':
		case '<':
		case '=':
		case '*':
		case '+':
		case '-':
		case '&':
		case '|':
			tokens.push_back(lex_consume_ambiguous_lexeme());
			break;
		case '"':
			tokens.push_back(lex_consume_string_literal());
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
	case '.':
		token.type = TokenType::dot;
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

Token
Lexer::lex_consume_ambiguous_lexeme()
{
	Token token{};
	token.start = &input_[cursor_];
	token.size = 1;
	token.neighborhood.line_num = curr_line_;

	char c = input_[cursor_];

	switch( c )
	{
	case '.':
		if( peek("..") )
		{
			return new_token(TokenType::var_args, 3);
		}
		else
		{
			return new_token(TokenType::dot, 1);
		}
	case '&':
		if( peek("&") )
		{
			return new_token(TokenType::and_and_lex, 2);
		}
		else
		{
			return new_token(TokenType::and_lex, 1);
		}
		break;
	case '|':
		if( peek("|") )
		{
			return new_token(TokenType::or_or_lex, 2);
		}
		else
		{}
		break;
	case '!':
		if( peek("=") )
		{
			return new_token(TokenType::ne, 2);
		}
		else
		{
			token.type = TokenType::exclam;
		}
		break;
	case '/':
		if( peek("/") )
		{
			return lex_consume_line_comment();
		}
		else if( peek("=") )
		{
			return new_token(TokenType::div_equal, 2);
		}
		else
		{
			token.type = TokenType::slash;
		}
		break;
	case '*':
		if( peek("=") )
		{
			return new_token(TokenType::mul_equal, 2);
		}
		else
		{
			token.type = TokenType::star;
		}
		break;
	case '+':
		if( peek("=") )
		{
			return new_token(TokenType::plus_equal, 2);
		}
		else
		{
			token.type = TokenType::plus;
		}
		break;
	case '-':
		if( peek("=") )
		{
			return new_token(TokenType::sub_equal, 2);
		}
		else if( peek(">") )
		{
			return new_token(TokenType::indirect_member_access, 2);
		}
		else
		{
			token.type = TokenType::minus;
		}
		break;

	case '=':
		if( peek("=") )
		{
			return new_token(TokenType::cmp, 2);
		}
		else
		{
			token.type = TokenType::equal;
		}
		break;
	case '>':
		if( peek("=") )
		{
			return new_token(TokenType::gte, 2);
		}
		else
		{
			token.type = TokenType::gt;
		}
		break;
	case '<':
		if( peek("=") )
		{
			return new_token(TokenType::lte, 2);
		}
		else
		{
			token.type = TokenType::lt;
		}
		break;
	default:
		token.type = TokenType::bad;
		break;
	}

done:
	return token;
}

Token
Lexer::lex_consume_string_literal()
{
	Token token{};
	token.type = TokenType::literal;
	token.literal_type = LiteralType::string;
	token.start = &input_[cursor_];
	token.size = 1;
	token.neighborhood.line_num = curr_line_;
	cursor_ += 1;

	char c = input_[cursor_];
	while( c != '"' )
	{
		token.size += 1;
		cursor_ += 1;
		c = input_[cursor_];
	}
	token.size += 1;

done:
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
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}

Token
Lexer::new_token(TokenType token_type, int size)
{
	Token token{token_type};
	token.start = &input_[cursor_];
	token.size = size;
	token.neighborhood.line_num = curr_line_;

	cursor_ += size - 1;
	return token;
}