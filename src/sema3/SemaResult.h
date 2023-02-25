
#pragma once

#include "lexer/token.h"

#include <iostream>
#include <optional>
#include <string>

namespace sema
{

class SemaError
{
public:
	std::string error;
	Token token;

	SemaError(char const* str)
		: error(str){};
	SemaError(std::string& str)
		: error(std::move(str)){};
	SemaError(std::string&& str)
		: error(std::move(str)){};

	SemaError(char const* str, Token token)
		: error(str)
		, token(token){};
	SemaError(std::string& str, Token token)
		: error(str)
		, token(token){};
	SemaError(std::string&& str, Token token)
		: error(str)
		, token(token){};

	void print() const;
};

template<typename T>
class SemaResult
{
	std::unique_ptr<SemaError> error = std::unique_ptr<SemaError>(nullptr);
	std::optional<T> result;

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
		: error(new SemaError(std::move(err))){};

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

	T unwrap() { return result.value(); }
	std::unique_ptr<SemaError> unwrap_error() { return std::move(error); }

	bool ok() const { return error.get() == nullptr; }

	static SemaResult<T> Ok(T el) { return SemaResult(el); }
};
} // namespace sema