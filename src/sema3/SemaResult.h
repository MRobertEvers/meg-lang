
#pragma once

#include "ast3/AstNode.h"

#include <memory>
#include <optional>
#include <string>

class SemaError
{
public:
	std::string error;
	AstNode* node;

	SemaError(char const* str)
		: error(str){};
	SemaError(std::string& str)
		: error(std::move(str)){};
	SemaError(std::string&& str)
		: error(std::move(str)){};

	SemaError(char const* str, AstNode* token)
		: error(str)
		, node(token){};
	SemaError(std::string& str, AstNode* token)
		: error(str)
		, node(token){};
	SemaError(std::string&& str, AstNode* token)
		: error(str)
		, node(token){};

	void print() const;
};

template<typename T>
class SemaResult
{
	std::unique_ptr<SemaError> error = nullptr;
	std::optional<T> result;

public:
	SemaResult<T>(T val)
		: result(val)
		, error(nullptr){
			  // std::cout << "ParseResult: " << std::hex << &val << std::endl;
		  };

	SemaResult<T>(SemaError err)
		: error(std::make_unique<SemaError>(err)){};

	T unwrap() { return result.value(); }
	std::unique_ptr<SemaError> unwrap_error() { return std::move(error); }

	bool ok() const { return error.get() == nullptr && result.has_value(); }
};