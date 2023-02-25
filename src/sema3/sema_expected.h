#pragma once

#include "SemaResult.h"
#include "ast2/Ast.h"
#include "ast2/AstCasts.h"

namespace sema
{

template<typename NodeType>
SemaResult<NodeType>
expected(ast::AstNode* node, ast::Cast<NodeType> (*cast)(ast::AstNode* node))
{
	auto castr = cast(node);
	if( !castr.ok() )
		return SemaError(
			"Expected type '" + ast::to_string(NodeType::nt) + "'. Received '" +
			ast::to_string(node->type) + "'.");

	return SemaResult(castr.unwrap());
}

} // namespace sema
