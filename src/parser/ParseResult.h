
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

	ParseResult(OwnPtr<T>& ptr)
		: result(std::move(ptr)){};
	ParseResult(OwnPtr<T>&& ptr)
		: result(std::move(ptr)){};

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

	/**
	 * @brief For passing up Concrete type to Base Type,
	 *
	 * e.g.
	 *
	 * ParseResult<IBase>
	 * parse() {
	 * 		return Concrete{};
	 * }
	 *
	 * @tparam TOtherParseResult
	 * @param other
	 */
	template<
		typename TPolymorphic,
		typename = std::enable_if_t<std::is_base_of<T, TPolymorphic>::value>>
	ParseResult(TPolymorphic&& other)
		: result(new TPolymorphic(std::move(other))){

		  };

	/**
	 * @brief For passing up ParseResults of Concrete type to Base Type,
	 *
	 * e.g. return ParseResult<ConcreteExpr> ->
	 * ParseResult<IExpressionNode>
	 *
	 * ParseResult<Concrete>
	 * parse_inner() {
	 * 		return Concrete{}; // type Concrete
	 * }
	 *
	 * ParseResult<IExpressionNode>
	 * parse() {
	 * 		auto concrete_parse_result = parse_inner();
	 * 		return concrete_parse_result; // type: ParseResult<Concrete>
	 * }
	 *
	 * @tparam TOther
	 * @param other
	 */
	template<typename TOther, typename = std::enable_if_t<std::is_base_of<T, TOther>::value>>
	ParseResult<T>(ParseResult<TOther>&& other)
		: result(std::move(other.unwrap()))
		, error(std::move(other.unwrap_error())){};

	/**
	 * @brief For passing errors.
	 * If this is somehow used to pass up a non-compatible non-error parse result,
	 * assert will throw.
	 *
	 * Strips the error out of the other parse result.
	 *
	 * ParseResult<IncompatibleType>
	 * parse_inner() {
	 * 		return ParseError("Bad parse");
	 * }
	 *
	 * ParseResult<MyType>
	 * parse() {
	 * 		auto incompatible_parse_result = parse_inner();
	 * 		if (!incompatible_parse_result.ok()) {
	 * 			return incompatible_parse_result; // type ParseResult<IncompatibleType>
	 * 		}
	 * }
	 *
	 * @tparam TOtherParseResult
	 * @param other
	 */
	template<
		typename TOther,
		typename = std::enable_if_t<!std::is_base_of<T, TOther>::value>,
		typename = void>
	ParseResult<T>(ParseResult<TOther>&& other)
		: error(std::move(other.unwrap_error()))
	{
		assert(
			other.unwrap().is_null() &&
			"Attempted to pass non-error result through non-polymorphic ParseResult. "
			"Did you try returning a Statement from something expecting an Expression or "
			"vice-versa?");
	};

	void reset() { result.reset(); }

	OwnPtr<T> unwrap() { return std::move(result); }
	OwnPtr<ParseError> unwrap_error() { return std::move(error); }

	T const* as() const { return result.get(); }
	bool ok() const { return error.is_null(); }
};