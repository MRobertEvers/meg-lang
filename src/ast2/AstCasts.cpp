
#include "AstCasts.h"

using namespace ast;

Cast<AstFn>
ast::as_fn(ast::AstNode* node)
{
	if( node->type != AstFn::nt )
		return Cast<AstFn>();

	return &node->data.fn;
}

Cast<AstExternFn>
ast::as_extern_fn(ast::AstNode* node)
{
	if( node->type != AstExternFn::nt )
		return Cast<AstExternFn>();

	return &node->data.extern_fn;
}

Cast<AstModule>
ast::as_module(ast::AstNode* node)
{
	if( node->type != AstModule::nt )
		return Cast<AstModule>();

	return &node->data.mod;
}

Cast<AstIf>
ast::as_if(ast::AstNode* node)
{
	//
	if( node->type != AstIf::nt )
		return Cast<AstIf>();

	return &node->data.ifcond;
}

Cast<AstElse>
ast::as_else(ast::AstNode* node)
{
	//
	if( node->type != AstElse::nt )
		return Cast<AstElse>();

	return &node->data.else_stmt;
}

Cast<AstFnProto>
ast::as_fn_proto(ast::AstNode* node)
{
	if( node->type != AstFnProto::nt )
		return Cast<AstFnProto>();

	return &node->data.fn_proto;
}

Cast<AstFnParamList>
ast::as_fn_param_list(ast::AstNode* node)
{
	if( node->type != AstFnParamList::nt )
		return Cast<AstFnParamList>();

	return &node->data.fn_params;
}

Cast<AstExprList>
ast::as_expr_list(ast::AstNode* node)
{
	if( node->type != AstExprList::nt )
		return Cast<AstExprList>();

	return &node->data.expr_list;
}

Cast<AstReturn>
ast::as_fn_return(ast::AstNode* node)
{
	if( node->type != AstReturn::nt )
		return Cast<AstReturn>();

	return &node->data.returnexpr;
}

Cast<AstLet>
ast::as_let(ast::AstNode* node)
{
	if( node->type != AstLet::nt )
		return Cast<AstLet>();

	return &node->data.let;
}

Cast<AstAssign>
ast::as_assign(ast::AstNode* node)
{
	if( node->type != AstAssign::nt )
		return Cast<AstAssign>();

	return &node->data.assign;
}

Cast<AstBinOp>
ast::as_binop(ast::AstNode* node)
{
	if( node->type != AstBinOp::nt )
		return Cast<AstBinOp>();

	return &node->data.binop;
}

Cast<AstId>
ast::as_id(ast::AstNode* node)
{
	if( node->type != AstId::nt )
		return Cast<AstId>();

	return &node->data.id;
}

Cast<AstBlock>
ast::as_block(ast::AstNode* node)
{
	if( node->type != AstBlock::nt )
		return Cast<AstBlock>();

	return &node->data.block;
}

Cast<AstTypeDeclarator>
ast::as_type_decl(ast::AstNode* node)
{
	if( node->type != AstTypeDeclarator::nt )
		return Cast<AstTypeDeclarator>();

	return &node->data.type_declarator;
}

Cast<AstValueDecl>
ast::as_value_decl(ast::AstNode* node)
{
	if( node->type != AstValueDecl::nt )
		return Cast<AstValueDecl>();

	return &node->data.value_decl;
}

Cast<AstFnCall>
ast::as_fn_call(ast::AstNode* node)
{
	if( node->type != AstFnCall::nt )
		return Cast<AstFnCall>();

	return &node->data.fn_call;
}

Cast<AstNumberLiteral>
ast::as_number_literal(ast::AstNode* node)
{
	if( node->type != AstNumberLiteral::nt )
		return Cast<AstNumberLiteral>();

	return &node->data.number_literal;
}

Cast<AstStringLiteral>
ast::as_string_literal(ast::AstNode* node)
{
	if( node->type != AstStringLiteral::nt )
		return Cast<AstStringLiteral>();

	return &node->data.string_literal;
}

Cast<AstStmt>
ast::as_stmt(ast::AstNode* node)
{
	if( node->type != AstStmt::nt )
		return Cast<AstStmt>();

	return &node->data.stmt;
}

Cast<AstExpr>
ast::as_expr(ast::AstNode* node)
{
	if( node->type != AstExpr::nt )
		return Cast<AstExpr>();

	return &node->data.expr;
}
Cast<AstStruct>
ast::as_struct(ast::AstNode* node)
{
	if( node->type != AstStruct::nt )
		return Cast<AstStruct>();

	return &node->data.structstmt;
}

Cast<AstAddressOf>
ast::as_address_of(ast::AstNode* node)
{
	if( node->type != AstAddressOf::nt )
		return Cast<AstAddressOf>();

	return &node->data.address_of;
}

Cast<AstDeref>
ast::as_deref(ast::AstNode* node)
{
	if( node->type != AstDeref::nt )
		return Cast<AstDeref>();

	return &node->data.deref;
}

Cast<AstMemberAccess>
ast::as_member_access(ast::AstNode* node)
{
	if( node->type != AstMemberAccess::nt )
		return Cast<AstMemberAccess>();

	return &node->data.member_access;
}

Cast<AstIndirectMemberAccess>
ast::as_indirect_member_access(ast::AstNode* node)
{
	if( node->type != AstIndirectMemberAccess::nt )
		return Cast<AstIndirectMemberAccess>();

	return &node->data.indirect_member_access;
}

Cast<AstEmpty>
ast::as_empty(ast::AstNode* node)
{
	if( node->type != AstEmpty::nt )
		return Cast<AstEmpty>();

	return &node->data.empty;
}

Cast<AstFor>
ast::as_for(ast::AstNode* node)
{
	if( node->type != AstFor::nt )
		return Cast<AstFor>();

	return &node->data.forstmt;
}

Cast<AstWhile>
ast::as_while(ast::AstNode* node)
{
	if( node->type != AstWhile::nt )
		return Cast<AstWhile>();

	return &node->data.whilestmt;
}

Cast<AstArrayAccess>
ast::as_array_acess(ast::AstNode* node)
{
	if( node->type != AstArrayAccess::nt )
		return Cast<AstArrayAccess>();

	return &node->data.array_access;
}

Cast<AstUnion>
ast::as_union(ast::AstNode* node)
{
	if( node->type != AstUnion::nt )
		return Cast<AstUnion>();

	return &node->data.unionstmt;
}

Cast<AstEnum>
ast::as_enum(ast::AstNode* node)
{
	if( node->type != AstEnum::nt )
		return Cast<AstEnum>();

	return &node->data.enumstmt;
}

Cast<AstEnumMember>
ast::as_enum_member(ast::AstNode* node)
{
	if( node->type != AstEnumMember::nt )
		return Cast<AstEnumMember>();

	return &node->data.enum_member;
}

Cast<AstIfArrow>
ast::as_if_arrow(ast::AstNode* node)
{
	if( node->type != AstIfArrow::nt )
		return Cast<AstIfArrow>();

	return &node->data.if_arrow;
}

Cast<AstIs>
ast::as_is(ast::AstNode* node)
{
	if( node->type != AstIs::nt )
		return Cast<AstIs>();

	return &node->data.is;
}

Cast<AstInitializer>
ast::as_initializer(ast::AstNode* node)
{
	//
	if( node->type != AstInitializer::nt )
		return Cast<AstInitializer>();

	return &node->data.initializer;
}

Cast<AstInitializerDesignator>
ast::as_initializer_designator(ast::AstNode* node)
{
	//
	if( node->type != AstInitializerDesignator::nt )
		return Cast<AstInitializerDesignator>();

	return &node->data.designator;
}
