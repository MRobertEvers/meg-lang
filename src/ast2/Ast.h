#pragma once
#include "AstNode.h"
#include "AstTags.h"
#include "Span.h"
#include "common/String.h"
#include "common/Vec.h"

namespace ast
{

// TODO: AST owns all ast nodes.
// Track all memory allocs
class Ast
{
	AstTags tags;

public:
	Ast(){};

	template<
		typename T,
		typename = std::enable_if_t<std::is_base_of<IAstTag, typename T::TagType>::value>>
	typename T::TagType* query(AstNode* node);

private:
	template<typename T>
	AstNode* make_empty(Span span);

public:
	String* create_string(char const* cstr, unsigned int size);
	AstList<AstNode*>* create_list();

	AstNode* Module(Span span, AstList<AstNode*>* params);
	AstNode* ExternFn(Span span, AstNode* prototype);
	AstNode* Fn(Span span, AstNode* prototype, AstNode* body);
	AstNode* FnProto(Span span, AstNode* name, AstNode* params, AstNode* return_type);
	AstNode* FnParamList(Span span, AstList<AstNode*>* params);
	AstNode* ValueDecl(Span span, AstNode* name, AstNode* type_name);
	AstNode* FnCall(Span span, AstNode* call_target, AstNode* args);
	AstNode* ArrayAccess(Span span, AstNode* array_target, AstNode* expr);
	AstNode* ExprList(Span span, AstList<AstNode*>* args);
	AstNode* Block(Span span, AstList<AstNode*>* statements);
	AstNode* BinOp(Span span, BinOp op, AstNode* left, AstNode* right);
	AstNode* Id(Span span, IdClassification classification, String* name);
	AstNode* TypeId(Span span, String* name);
	AstNode* ValueId(Span span, String* name);
	AstNode* Assign(Span span, AssignOp op, AstNode* left, AstNode* right);
	AstNode* If(Span span, AstNode* condition, AstNode* then_block, AstNode* else_block);
	AstNode* Else(Span span, AstNode* stmt);
	AstNode* Let(Span span, AstNode* identifier, AstNode* type_declarator, AstNode* rhs);
	AstNode* Return(Span span, AstNode* expr);
	AstNode* Struct(Span span, AstNode* type_name, AstList<AstNode*>* members);
	AstNode* Union(Span span, AstNode* type_name, AstList<AstNode*>* members);
	// AstNode* Member(Span span, AstNode* identifier, AstNode* type_declarator);
	AstNode* While(Span span, AstNode* condition, AstNode* block);
	AstNode* For(Span span, AstNode* init, AstNode* condition, AstNode* end_loop, AstNode* body);
	AstNode* StringLiteral(Span span, String* literal);
	AstNode* NumberLiteral(Span span, long long literal);
	AstNode* TypeDeclarator(Span span, String* name, unsigned int indirection_level);
	AstNode* TypeDeclaratorArray(Span span, String* name, unsigned int, unsigned int);
	AstNode* TypeDeclaratorEmpty();
	AstNode* MemberAccess(Span span, AstNode* expr, AstNode* member_name);
	AstNode* IndirectMemberAccess(Span span, AstNode* expr, AstNode* member_name);
	AstNode* Deref(Span span, AstNode* expr);
	AstNode* AddressOf(Span span, AstNode* expr);
	AstNode* Expr(Span span, AstNode* expr);
	AstNode* Stmt(Span span, AstNode* expr);
	AstNode* VarArg(Span span);
	AstNode* Empty(Span span);
};

template<typename T, typename Enable>
typename T::TagType*
Ast::query(AstNode* node)
{
	return tags.query<T>(node);
}

template<typename T>
AstNode*
Ast::make_empty(Span span)
{
	auto node = new AstNode;
	node->type = T::nt;
	node->span = span;

	return node;
}

} // namespace ast
