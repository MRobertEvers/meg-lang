

#pragma once

#include "IR.h"
#include "Sema2.h"
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
		return SemaError("Expected type '" + ast::to_string(node->type) + "'");

	return SemaResult(castr.unwrap());
}

SemaResult<ir::IRModule*> sema_module(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRTopLevelStmt*> sema_tls(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRExternFn*> sema_extern_fn(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRProto*> sema_fn_proto(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRValueDecl*> sema_value_decl(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRTypeDeclaraor*> sema_type_decl(Sema2& sema, ast::AstNode* ast);

}; // namespace sema