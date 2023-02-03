
#pragma once

#include "common/OwnPtr.h"
#include "common/String.h"
#include "lexer/token.h"

#include <iostream>
namespace cg
{

class CGError
{
public:
	String error;
	Token token;

	CGError(char const* str)
		: error(str){};
	CGError(String& str)
		: error(std::move(str)){};
	CGError(String&& str)
		: error(std::move(str)){};

	CGError(char const* str, Token token)
		: error(str)
		, token(token){};
	CGError(String& str, Token token)
		: error(str)
		, token(token){};
	CGError(String&& str, Token token)
		: error(str)
		, token(token){};

	void print() const;
};

template<typename T>
class CGResult
{
	OwnPtr<CGError> error = OwnPtr<CGError>::null();
	T result;

public:
	CGResult<T>(T const& val)
		: result(val){
			  // std::cout << "CGResult: " << std::hex << &val << std::endl;
		  };
	CGResult<T>(T& val)
		: result(std::move(val)){
			  // std::cout << "CGResult: " << std::hex << &val << std::endl;
		  };
	CGResult<T>(T&& val)
		: result(std::move(val)){
			  // std::cout << "CGResult: " << std::hex << &val << std::endl;
		  };

	/**
	 * @brief Construct a new CGResult<T> object from an error
	 *
	 * CGResult<ResultType>
	 * sema() {
	 * 		return CGResult("Bad parse");
	 * }
	 *
	 * @param err
	 */
	CGResult<T>(CGError err)
		: error(err){};

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
	CGResult<T>(CGResult<TOther>&& other)
		: error(std::move(other.unwrap_error()))
	{
		assert(
			other.ok() &&
			"Attempted to pass non-error result through non-polymorphic ParseResult. "
			"Did you try returning a Statement from something expecting an Expression or "
			"vice-versa?");
	};

	T unwrap() { return result; }
	OwnPtr<CGError> unwrap_error() { return std::move(error); }

	bool ok() const { return error.is_null(); }

	static CGResult<T> Ok(T el) { return CGResult(el); }
};
} // namespace cg