
#pragma once

#include "common/OwnPtr.h"
#include "common/String.h"
#include "lexer/token.h"

#include <iostream>
namespace sema
{

class SemaError
{
public:
	String error;
	Token token;

	SemaError(char const* str)
		: error(str){};
	SemaError(String& str)
		: error(std::move(str)){};
	SemaError(String&& str)
		: error(std::move(str)){};

	SemaError(char const* str, Token token)
		: error(str)
		, token(token){};
	SemaError(String& str, Token token)
		: error(str)
		, token(token){};
	SemaError(String&& str, Token token)
		: error(str)
		, token(token){};

	void print() const;
};

template<typename T>
class SemaResult
{
	OwnPtr<SemaError> error = OwnPtr<SemaError>::null();
	T result;

public:
	SemaResult<T>(T const& val)
		: result(val){
			  // std::cout << "SemaResult: " << std::hex << &val << std::endl;
		  };
	SemaResult<T>(T& val)
		: result(std::move(val)){
			  // std::cout << "SemaResult: " << std::hex << &val << std::endl;
		  };
	SemaResult<T>(T&& val)
		: result(std::move(val)){
			  // std::cout << "SemaResult: " << std::hex << &val << std::endl;
		  };

	/**
	 * @brief Construct a new SemaResult<T> object from an error
	 *
	 * SemaResult<ResultType>
	 * sema() {
	 * 		return SemaResult("Bad parse");
	 * }
	 *
	 * @param err
	 */
	SemaResult<T>(SemaError err)
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
	SemaResult<T>(SemaResult<TOther>&& other)
		: error(std::move(other.unwrap_error()))
	{
		assert(
			other.ok() &&
			"Attempted to pass non-error result through non-polymorphic ParseResult. "
			"Did you try returning a Statement from something expecting an Expression or "
			"vice-versa?");
	};

	T unwrap() { return result; }
	OwnPtr<SemaError> unwrap_error() { return std::move(error); }

	bool ok() const { return error.is_null(); }

	static SemaResult<T> Ok(T el) { return SemaResult(el); }
};
} // namespace sema