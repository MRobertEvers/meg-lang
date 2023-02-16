

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
		return SemaError(
			"Expected type '" + ast::to_string(NodeType::nt) + "'. Received '" +
			ast::to_string(node->type) + "'.");

	return SemaResult(castr.unwrap());
}

SemaResult<ir::IRModule*> sema_module(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRTopLevelStmt*> sema_tls(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRStmt*> sema_stmt(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRIf*> sema_if(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRElse*> sema_else(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRValueDecl*> sema_struct_tls(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRExpr*> sema_expr_any(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRExpr*> sema_expr(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRExternFn*> sema_extern_fn(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRFunction*> sema_fn(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRArgs*> sema_fn_args(Sema2& sema, ast::AstNode* ast, sema::Type const& fn_type);
SemaResult<ir::IRCall*> sema_fn_call(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRId*> sema_id(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRMemberAccess*> sema_member_access(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRIndirectMemberAccess*> sema_indirect_member_access(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRAddressOf*> sema_addressof(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRDeref*> sema_deref(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRReturn*> sema_return(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRLet*> sema_let(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRAssign*> sema_assign(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRBinOp*> sema_binop(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRBlock*> sema_block(Sema2& sema, ast::AstNode* ast, bool new_scope);
SemaResult<ir::IRParam*> sema_fn_param(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRProto*> sema_fn_proto(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRStruct*> sema_struct(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRValueDecl*> sema_value_decl(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRNumberLiteral*> sema_number_literal(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRStringLiteral*> sema_string_literal(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRTypeDeclaraor*> sema_type_decl(Sema2& sema, ast::AstNode* ast);

SemaResult<ir::IRFunction*> generate_constructor(Sema2& sema, ast::AstNode* ast);

}; // namespace sema