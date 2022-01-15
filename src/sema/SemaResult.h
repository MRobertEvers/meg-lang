
#pragma once

#include "common/OwnPtr.h"
#include "common/String.h"
#include "lexer/token.h"

#include <iostream>

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
	OwnPtr<T> result = OwnPtr<T>::null();

public:
	SemaResult<T>(T&& val)
		: result(std::move(val)){
			  // std::cout << "SemaResult: " << std::hex << &val << std::endl;
		  };

	SemaResult(OwnPtr<T>& ptr)
		: result(std::move(ptr)){};
	SemaResult(OwnPtr<T>&& ptr)
		: result(std::move(ptr)){};

	/**
	 * @brief Construct a new SemaResult<T> object from an error
	 *
	 * SemaResult<ResultType>
	 * parse_inner() {
	 * 		return SemaError("Bad parse");
	 * }
	 *
	 * @param err
	 */
	SemaResult<T>(SemaError err)
		: error(err){};

	/**
	 * @brief For passing up Concrete type to Base Type,
	 *
	 * e.g.
	 *
	 * SemaResult<IBase>
	 * parse() {
	 * 		return Concrete{};
	 * }
	 *
	 * @tparam TOtherSemaResult
	 * @param other
	 */
	template<
		typename TPolymorphic,
		typename = std::enable_if_t<std::is_base_of<T, TPolymorphic>::value>>
	SemaResult(TPolymorphic&& other)
		: result(new TPolymorphic(std::move(other))){

		  };

	/**
	 * @brief For passing up SemaResults of Concrete type to Base Type,
	 *
	 * e.g. return SemaResult<ConcreteExpr> ->
	 * SemaResult<IExpressionNode>
	 *
	 * SemaResult<Concrete>
	 * parse_inner() {
	 * 		return Concrete{}; // type Concrete
	 * }
	 *
	 * SemaResult<IExpressionNode>
	 * parse() {
	 * 		auto concrete_parse_result = parse_inner();
	 * 		return concrete_parse_result; // type: SemaResult<Concrete>
	 * }
	 *
	 * @tparam TOther
	 * @param other
	 */
	template<typename TOther, typename = std::enable_if_t<std::is_base_of<T, TOther>::value>>
	SemaResult<T>(SemaResult<TOther>&& other)
		: result(std::move(other.unwrap()))
		, error(std::move(other.unwrap_error())){};

	/**
	 * @brief For passing errors.
	 * If this is somehow used to pass up a non-compatible non-error parse result,
	 * assert will throw.
	 *
	 * Strips the error out of the other parse result.
	 *
	 * SemaResult<IncompatibleType>
	 * parse_inner() {
	 * 		return SemaError("Bad parse");
	 * }
	 *
	 * SemaResult<MyType>
	 * parse() {
	 * 		auto incompatible_parse_result = parse_inner();
	 * 		if (!incompatible_parse_result.ok()) {
	 * 			return incompatible_parse_result; // type SemaResult<IncompatibleType>
	 * 		}
	 * }
	 *
	 * @tparam TOtherSemaResult
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
			other.unwrap().is_null() &&
			"Attempted to pass non-error result through non-polymorphic SemaResult. "
			"Did you try returning a Statement from something expecting an Expression or "
			"vice-versa?");
	};

	void reset() { result.reset(); }

	OwnPtr<T> unwrap() { return std::move(result); }
	OwnPtr<SemaError> unwrap_error() { return std::move(error); }

	T const* as() const { return result.get(); }
	bool ok() const { return error.is_null(); }
};