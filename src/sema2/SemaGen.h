

#pragma once

#include "IR.h"
#include "Sema2.h"
#include "SemaResult.h"
#include "ast2/Ast.h"
#include "ast2/AstCasts.h"

namespace sema
{

SemaResult<ir::IRModule*> sema_module(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRTopLevelStmt*> sema_tls(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRStmt*> sema_stmt(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRIf*> sema_if(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRIs*> sema_is(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRFor*> sema_for(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRWhile*> sema_while(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRElse*> sema_else(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRValueDecl*> sema_struct_tls(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRExpr*> sema_expr_any(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRExpr*> sema_expr(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRExternFn*> sema_extern_fn(Sema2& sema, ast::AstNode* ast);

struct sema_fn_t
{
	enum class Kind
	{
		Generator,
		Fn,
	} kind;
	union
	{
		ir::IRFunction* fn;
		ir::IRGenerator* generator;
	};

	sema_fn_t(ir::IRFunction* fn_)
		: fn(fn_)
		, kind(Kind::Fn)
	{}
	sema_fn_t(ir::IRGenerator* generator)
		: generator(generator)
		, kind(Kind::Generator)
	{}
};
SemaResult<sema_fn_t> sema_fn(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRArgs*> sema_fn_args(Sema2& sema, ast::AstNode* ast, sema::Type const& fn_type);
SemaResult<ir::IRCall*> sema_fn_call(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRArrayAccess*> sema_array_access(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRMemberAccess*> sema_member_access(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRIndirectMemberAccess*> sema_indirect_member_access(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRAddressOf*> sema_addressof(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRDeref*> sema_deref(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRReturn*> sema_return(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRLet*> sema_let(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRSwitch*> sema_switch(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRCase*> sema_case(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRYield*> sema_yield(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRAssign*> sema_assign(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRBinOp*> sema_binop(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRBlock*> sema_block(Sema2& sema, ast::AstNode* ast, bool new_scope);
SemaResult<ir::IRParam*> sema_fn_param(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRProto*> sema_fn_proto(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRStruct*> sema_struct(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRInitializer*> sema_initializer(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRUnion*> sema_union(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IREnum*> sema_enum(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IREnumMember*>
sema_enum_member(Sema2& sema, String const&, ast::AstNode* ast, Type const*, EnumNominal idx);
SemaResult<ir::IRValueDecl*> sema_value_decl(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRNumberLiteral*> sema_number_literal(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRStringLiteral*> sema_string_literal(Sema2& sema, ast::AstNode* ast);
SemaResult<ir::IRTypeDeclaraor*> sema_type_decl(Sema2& sema, ast::AstNode* ast);

SemaResult<ir::IRFunction*> generate_constructor(Sema2& sema, ast::AstNode* ast);

}; // namespace sema