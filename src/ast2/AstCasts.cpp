
#include "AstCasts.h"

using namespace ast;

Cast<AstFn>
ast::as_fn(ast::AstNode* node)
{
	if( node->type != AstFn::nt )
		return Cast<AstFn>();

	return &node->data.fn;
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

Cast<AstReturn>
ast::as_fn_return(ast::AstNode* node)
{
	if( node->type != AstReturn::nt )
		return Cast<AstReturn>();

	return &node->data.returnexpr;
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