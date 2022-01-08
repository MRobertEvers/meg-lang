
#pragma once

#include "common/OwnPtr.h"
#include "common/String.h"

#include <iostream>

class ParseError
{
public:
	String error;

	ParseError(char const* str)
		: error(str){};
	ParseError(String& str)
		: error(std::move(str)){};
};

template<typename T>
class ParseResult
{
	OwnPtr<ParseError> error = OwnPtr<ParseError>::null();
	OwnPtr<T> result = OwnPtr<T>::null();

public:
	ParseResult<T>(ParseError err)
		: error(err){};

	ParseResult(OwnPtr<T> ptr)
		: result(std::move(ptr)){};

	/**
	 * @brief For passing up Base pointers, e.g. ParseResult<ConcreteExpr> ->
	 * ParseResult<IExpressionNode>
	 *
	 *
	 * @tparam TOtherParseResult
	 * @param other
	 */
	template<typename TOther, typename = std::enable_if_t<std::is_base_of<T, TOther>::value>>
	ParseResult<T>(ParseResult<TOther>& other)
		: result(other.unwrap().get())
		, error(std::move(other.get_error()))
	{
		std::cout << "Hello" << std::endl;
	};

	/**
	 * @brief For passing errors.
	 *
	 * @tparam TOtherParseResult
	 * @param other
	 */
	template<
		typename TOther,
		typename = std::enable_if_t<!std::is_base_of<T, TOther>::value>,
		typename = void>
	ParseResult<T>(ParseResult<TOther>& other)
		: error(std::move(other.get_error())){};

	OwnPtr<T> unwrap() { return std::move(result); }
	OwnPtr<ParseError>& get_error() { return error; }

	bool ok() const { return error.is_null(); }
};