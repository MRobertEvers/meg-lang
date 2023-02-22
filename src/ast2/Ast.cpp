

#include "Ast.h"

#include "bin_op.h"

using namespace ast;

String*
Ast::create_string(char const* cstr, unsigned int size)
{
	return new String{cstr, size};
}

AstList<String*>*
Ast::create_name_parts()
{
	return new AstList<String*>{};
}

AstList<AstNode*>*
Ast::create_list()
{
	return new AstList<AstNode*>{};
}

AstNode*
Ast::Module(Span span, AstList<AstNode*>* params)
{
	auto node = make_empty<AstModule>(span);
	node->data.mod = AstModule{params};
	return node;
}

AstNode*
Ast::ExternFn(Span span, AstNode* prototype)
{
	auto node = make_empty<AstExternFn>(span);
	node->data.extern_fn = AstExternFn{prototype};
	return node;
}

AstNode*
Ast::Fn(Span span, AstNode* prototype, AstNode* body)
{
	auto node = make_empty<AstFn>(span);
	node->data.fn = AstFn{prototype, body};
	return node;
}

AstNode*
Ast::FnProto(Span span, AstNode* name, AstNode* params, AstNode* return_type)
{
	auto node = make_empty<AstFnProto>(span);
	node->data.fn_proto = AstFnProto{name, params, return_type};
	return node;
}

AstNode*
Ast::FnParamList(Span span, AstList<AstNode*>* params)
{
	auto node = make_empty<AstFnParamList>(span);
	node->data.fn_params = AstFnParamList{params};
	return node;
}

AstNode*
Ast::ValueDecl(Span span, AstNode* name, AstNode* type_name)
{
	auto node = make_empty<AstValueDecl>(span);
	node->data.value_decl = AstValueDecl{name, type_name};
	return node;
}

AstNode*
Ast::FnCall(Span span, AstNode* call_target, AstNode* args)
{
	auto node = make_empty<AstFnCall>(span);
	node->data.fn_call = AstFnCall{call_target, args};
	return node;
}

AstNode*
Ast::ArrayAccess(Span span, AstNode* array_target, AstNode* expr)
{
	auto node = make_empty<AstArrayAccess>(span);
	node->data.array_access = AstArrayAccess{array_target, expr};
	return node;
}

AstNode*
Ast::ExprList(Span span, AstList<AstNode*>* args)
{
	auto node = make_empty<AstExprList>(span);
	node->data.expr_list = AstExprList{args};
	return node;
}

AstNode*
Ast::Block(Span span, AstList<AstNode*>* statements)
{
	auto node = make_empty<AstBlock>(span);
	node->data.block = AstBlock{statements};
	return node;
}

AstNode*
Ast::BinOp(Span span, enum BinOp op, AstNode* left, AstNode* right)
{
	auto node = make_empty<AstBinOp>(span);
	node->data.binop = AstBinOp{op, left, right};
	return node;
}

AstNode*
Ast::Id(Span span, AstList<String*>* name)
{
	auto node = make_empty<AstId>(span);
	node->data.id = AstId{IdClassification::ValueIdentifier, name};
	return node;
}

// AstNode*
// Ast::TypeId(Span span, String* name)
// {
// 	auto node = make_empty<AstId>(span);
// 	node->data.id = AstId{IdClassification::TypeIdentifier, name};
// 	return node;
// }

// AstNode*
// Ast::ValueId(Span span, Vec<String*>* name)
// {
// 	auto node = make_empty<AstId>(span);
// 	node->data.id = AstId{IdClassification::ValueIdentifier, name};
// 	return node;
// }

AstNode*
Ast::Assign(Span span, AssignOp op, AstNode* left, AstNode* right)
{
	auto node = make_empty<AstAssign>(span);
	node->data.assign = AstAssign{op, left, right};
	return node;
}

AstNode*
Ast::If(Span span, AstNode* condition, AstNode* then_block, AstNode* else_block)
{
	auto node = make_empty<AstIf>(span);
	node->data.ifcond = AstIf{condition, then_block, else_block};
	return node;
}

AstNode*
Ast::IfArrow(Span span, AstNode* params, AstNode* block)
{
	auto node = make_empty<AstIfArrow>(span);
	node->data.if_arrow = AstIfArrow{params, block};
	return node;
}

AstNode*
Ast::Else(Span span, AstNode* stmt)
{
	auto node = make_empty<AstElse>(span);
	node->data.else_stmt = AstElse{stmt};
	return node;
}

AstNode*
Ast::Let(Span span, AstNode* identifier, AstNode* type_declarator, AstNode* rhs)
{
	auto node = make_empty<AstLet>(span);
	node->data.let = AstLet{identifier, type_declarator, rhs};
	return node;
}

AstNode*
Ast::Return(Span span, AstNode* expr)
{
	auto node = make_empty<AstReturn>(span);
	node->data.returnexpr = AstReturn{expr};
	return node;
}

AstNode*
Ast::Struct(Span span, AstNode* type_name, AstList<AstNode*>* members)
{
	auto node = make_empty<AstStruct>(span);
	node->data.structstmt = AstStruct{type_name, members};
	return node;
}

AstNode*
Ast::Union(Span span, AstNode* type_name, AstList<AstNode*>* members)
{
	auto node = make_empty<AstUnion>(span);
	node->data.unionstmt = AstUnion{type_name, members};
	return node;
}

AstNode*
Ast::Enum(Span span, AstNode* type_name, AstList<AstNode*>* members)
{
	auto node = make_empty<AstEnum>(span);
	node->data.enumstmt = AstEnum{type_name, members};
	return node;
}

AstNode*
Ast::EnumMemberEmpty(Span span, String* name)
{
	auto node = make_empty<AstEnumMember>(span);
	node->data.enum_member = AstEnumMember{name};
	return node;
}

AstNode*
Ast::EnumMemberStruct(Span span, AstNode* member)
{
	auto node = make_empty<AstEnumMember>(span);
	node->data.enum_member = AstEnumMember{member};
	return node;
}

// AstNode*
// Ast::Member(Span span, AstNode* identifier, AstNode* type_declarator)
// {
// 	auto node = make_empty<AstMemberDef>(span);
// 	node->data.member = AstMemberDef{identifier, type_declarator};
// 	return node;
// }

AstNode*
Ast::While(Span span, AstNode* condition, AstNode* block)
{
	auto node = make_empty<AstWhile>(span);
	node->data.whilestmt = AstWhile{condition, block};
	return node;
}

AstNode*
Ast::For(Span span, AstNode* init, AstNode* condition, AstNode* end_loop, AstNode* body)
{
	auto node = make_empty<AstFor>(span);
	node->data.forstmt = AstFor{init, condition, end_loop, body};
	return node;
}

AstNode*
Ast::StringLiteral(Span span, String* literal)
{
	auto node = make_empty<AstStringLiteral>(span);
	node->data.string_literal = AstStringLiteral{literal};
	return node;
}

AstNode*
Ast::NumberLiteral(Span span, long long literal)
{
	auto node = make_empty<AstNumberLiteral>(span);
	node->data.number_literal = AstNumberLiteral{literal};
	return node;
}

AstNode*
Ast::TypeDeclarator(Span span, AstList<String*>* name, unsigned int indirection_level)
{
	auto node = make_empty<AstTypeDeclarator>(span);
	node->data.type_declarator = AstTypeDeclarator{name, indirection_level};
	node->data.type_declarator.empty = false;
	return node;
}

AstNode*
Ast::TypeDeclaratorArray(
	Span span, AstList<String*>* name, unsigned int indirection_level, unsigned int array_size)
{
	auto node = make_empty<AstTypeDeclarator>(span);
	node->data.type_declarator = AstTypeDeclarator{name, indirection_level, array_size};
	node->data.type_declarator.empty = false;
	return node;
}

AstNode*
Ast::TypeDeclaratorEmpty()
{
	auto node = make_empty<AstTypeDeclarator>(Span{});
	node->data.type_declarator = AstTypeDeclarator{};
	node->data.type_declarator.empty = true;
	return node;
}

AstNode*
Ast::MemberAccess(Span span, AstNode* expr, AstNode* member_name)
{
	auto node = make_empty<AstMemberAccess>(span);
	node->data.member_access = AstMemberAccess{expr, member_name};
	return node;
}

AstNode*
Ast::IndirectMemberAccess(Span span, AstNode* expr, AstNode* member_name)
{
	auto node = make_empty<AstIndirectMemberAccess>(span);
	node->data.indirect_member_access = AstIndirectMemberAccess{expr, member_name};
	return node;
}

AstNode*
Ast::Deref(Span span, AstNode* expr)
{
	auto node = make_empty<AstDeref>(span);
	node->data.deref = AstDeref{expr};
	return node;
}

AstNode*
Ast::AddressOf(Span span, AstNode* expr)
{
	auto node = make_empty<AstAddressOf>(span);
	node->data.address_of = AstAddressOf{expr};
	return node;
}

AstNode*
Ast::Expr(Span span, AstNode* expr)
{
	auto node = make_empty<AstExpr>(span);
	node->data.expr = AstExpr{expr};
	return node;
}

AstNode*
Ast::Stmt(Span span, AstNode* expr)
{
	auto node = make_empty<AstStmt>(span);
	node->data.stmt = AstStmt{expr};
	return node;
}

AstNode*
Ast::VarArg(Span span)
{
	auto node = make_empty<AstVarArg>(span);
	node->data.var_arg = AstVarArg{};
	return node;
}

AstNode*
Ast::Empty(Span span)
{
	auto node = make_empty<AstEmpty>(span);
	node->data.empty = AstEmpty{};
	return node;
}

AstNode*
Ast::Is(Span span, AstNode* expr, AstNode* type)
{
	auto node = make_empty<AstIs>(span);
	node->data.is = AstIs{expr, type};
	return node;
}

AstNode*
Ast::Switch(Span span, AstNode* expr, AstNode* block)
{
	auto node = make_empty<AstSwitch>(span);
	node->data.switch_stmt = AstSwitch{expr, block};
	return node;
}

AstNode*
Ast::Case(Span span, AstNode* expr, AstNode* stmt)
{
	auto node = make_empty<AstCase>(span);
	node->data.case_stmt = AstCase{expr, stmt};
	return node;
}

AstNode*
Ast::CaseDefault(Span span, AstNode* stmt)
{
	auto node = make_empty<AstCase>(span);
	node->data.case_stmt = AstCase{stmt};
	return node;
}

AstNode*
Ast::Initializer(Span span, AstNode* type_name, AstList<AstNode*>* members)
{
	//
	auto node = make_empty<AstInitializer>(span);
	node->data.initializer = AstInitializer{type_name, members};
	return node;
}

AstNode*
Ast::InitializerDesignator(Span span, AstNode* name, AstNode* expr)
{
	//
	auto node = make_empty<AstInitializerDesignator>(span);
	node->data.designator = AstInitializerDesignator{name, expr};
	return node;
}