
#pragma once

#include "lex3/Token.h"

#include <iostream>
#include <memory>
#include <optional>
#include <string>

class ParseError
{
public:
	std::string error;
	Token token;

	ParseError(char const* str)
		: error(str){};
	ParseError(std::string& str)
		: error(std::move(str)){};
	ParseError(std::string&& str)
		: error(std::move(str)){};

	ParseError(char const* str, Token token)
		: error(str)
		, token(token){};
	ParseError(std::string& str, Token token)
		: error(str)
		, token(token){};
	ParseError(std::string&& str, Token token)
		: error(str)
		, token(token){};

	void print() const;
};

template<typename T>
class ParseResult
{
	std::unique_ptr<ParseError> error = nullptr;
	std::optional<T> result;

public:
	ParseResult<T>(T val)
		: result(val)
		, error(nullptr){
			  // std::cout << "ParseResult: " << std::hex << &val << std::endl;
		  };

	/**
	 * @brief Construct a new Parse Result<T> object from an error
	 *
	 * ParseResult<ResultType>
	 * parse_inner() {
	 * 		return ParseError("Bad parse");
	 * }
	 *
	 * @param err
	 */
	ParseResult<T>(ParseError err)
		: error(std::make_unique<ParseError>(err)){};

	T unwrap() { return result.value(); }
	std::unique_ptr<ParseError> unwrap_error() { return std::move(error); }

	bool ok() const { return error.get() == nullptr && result.has_value(); }
};