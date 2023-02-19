#pragma once
#include "AstNode.h"

namespace ast
{

template<typename T>
struct Cast
{
	T* value;

	Cast(T* val)
		: value(val){};
	Cast()
		: value(nullptr){};

	bool ok() { return value != nullptr; }
	T& unwrap() { return *value; }
};

Cast<AstFn> as_fn(ast::AstNode* node);
Cast<AstExternFn> as_extern_fn(ast::AstNode* node);
Cast<AstModule> as_module(ast::AstNode* node);
Cast<AstIf> as_if(ast::AstNode* node);
Cast<AstElse> as_else(ast::AstNode* node);
Cast<AstFnProto> as_fn_proto(ast::AstNode* node);
Cast<AstFnParamList> as_fn_param_list(ast::AstNode* node);
Cast<AstExprList> as_expr_list(ast::AstNode* node);
Cast<AstReturn> as_fn_return(ast::AstNode* node);
Cast<AstLet> as_let(ast::AstNode* node);
Cast<AstAssign> as_assign(ast::AstNode* node);
Cast<AstBinOp> as_binop(ast::AstNode* node);
Cast<AstId> as_id(ast::AstNode* node);
Cast<AstBlock> as_block(ast::AstNode* node);
Cast<AstTypeDeclarator> as_type_decl(ast::AstNode* node);
Cast<AstValueDecl> as_value_decl(ast::AstNode* node);
Cast<AstFnCall> as_fn_call(ast::AstNode* node);
Cast<AstNumberLiteral> as_number_literal(ast::AstNode* node);
Cast<AstStringLiteral> as_string_literal(ast::AstNode* node);
Cast<AstStmt> as_stmt(ast::AstNode* node);
Cast<AstExpr> as_expr(ast::AstNode* node);
Cast<AstStruct> as_struct(ast::AstNode* node);
Cast<AstAddressOf> as_address_of(ast::AstNode* node);
Cast<AstDeref> as_deref(ast::AstNode* node);
Cast<AstMemberAccess> as_member_access(ast::AstNode* node);
Cast<AstIndirectMemberAccess> as_indirect_member_access(ast::AstNode* node);
Cast<AstEmpty> as_empty(ast::AstNode* node);
Cast<AstFor> as_for(ast::AstNode* node);
Cast<AstWhile> as_while(ast::AstNode* node);
Cast<AstArrayAccess> as_array_acess(ast::AstNode* node);
Cast<AstUnion> as_union(ast::AstNode* node);
Cast<AstEnum> as_enum(ast::AstNode* node);
Cast<AstEnumMember> as_enum_member(ast::AstNode* node);
Cast<AstIfArrow> as_if_arrow(ast::AstNode* node);
Cast<AstIs> as_is(ast::AstNode* node);

// template<template<typename> typename Container, typename NodeType>
// Container<NodeType>
// expected(ast::AstNode* node, Cast<NodeType> (*cast)(ast::AstNode* node))
// {
// 	auto castr = cast(node);
// 	if( !castr.ok() )
// 		return Container("Expected type '" + ast::to_string(node->type) + "'");

// 	return castr.unwrap();
// }
}; // namespace ast