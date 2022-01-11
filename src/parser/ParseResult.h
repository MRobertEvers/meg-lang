
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
	OwnPtr<T> result = OwnPtr<T>::null();

public:
	ParseResult<T>(T&& val)
		: result(std::move(val)){
			  // std::cout << "ParseResult: " << std::hex << &val << std::endl;
		  };

	ParseResult<T>(ParseError err)
		: error(err){};

	// ParseResult(OwnPtr<T> ptr)
	// 	: result(std::move(ptr)){};
	ParseResult(OwnPtr<T>& ptr)
		: result(std::move(ptr)){};
	ParseResult(OwnPtr<T>&& ptr)
		: result(std::move(ptr)){};

	// ~ParseResult() { std::cout << "~ParseResult: " << std::hex << result.get() << std::endl; }

	template<
		typename TPolymorphic,
		typename = std::enable_if_t<std::is_base_of<T, TPolymorphic>::value>>
	ParseResult(TPolymorphic&& other)
		: result(new TPolymorphic(std::move(other))){

		  };

	/**
	 * @brief For passing up Base pointers, e.g. ParseResult<ConcreteExpr> ->
	 * ParseResult<IExpressionNode>
	 *
	 *
	 * @tparam TOtherParseResult
	 * @param other
	 */
	template<typename TOther, typename = std::enable_if_t<std::is_base_of<T, TOther>::value>>
	ParseResult<T>(ParseResult<TOther>&& other)
		: result(std::move(other.unwrap()))
		, error(std::move(other.get_error()))
	{
		std::cout << "Hello" << std::endl;
	};

	/**
	 * @brief For passing errors.
	 *
	 * TODO: Do I want this?
	 *
	 * @tparam TOtherParseResult
	 * @param other
	 */
	template<
		typename TOther,
		typename = std::enable_if_t<!std::is_base_of<T, TOther>::value>,
		typename = void>
	ParseResult<T>(ParseResult<TOther>&& other)
		: error(std::move(other.get_error()))
	{
		assert(
			other.unwrap().is_null() &&
			"Attempted to pass non-error result through non-polymorphic ParseResult. "
			"Did you try returning a Statement from something expecting an Expression or "
			"vice-versa?");
	};

	void reset() { result.reset(); }

	OwnPtr<T> unwrap() { return std::move(result); }
	OwnPtr<ParseError>& get_error() { return error; }
	T* as() { return result.get(); }

	bool ok() const { return error.is_null(); }
};