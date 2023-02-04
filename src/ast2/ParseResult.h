
#pragma once

#include "common/OwnPtr.h"
#include "common/String.h"
#include "lexer/token.h"

#include <iostream>

class ParseError
{
public:
	String error;
	Token token;

	ParseError(char const* str)
		: error(str){};
	ParseError(String& str)
		: error(std::move(str)){};
	ParseError(String&& str)
		: error(std::move(str)){};

	ParseError(char const* str, Token token)
		: error(str)
		, token(token){};
	ParseError(String& str, Token token)
		: error(str)
		, token(token){};
	ParseError(String&& str, Token token)
		: error(str)
		, token(token){};

	void print() const;
};

template<typename T>
class ParseResult
{
	OwnPtr<ParseError> error = OwnPtr<ParseError>::null();
	T result = nullptr;

public:
	ParseResult<T>(T val)
		: result(val){
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
		: error(err){};

	T unwrap() { return result; }
	OwnPtr<ParseError> unwrap_error() { return std::move(error); }

	bool ok() const { return error.is_null() && result != nullptr; }
};