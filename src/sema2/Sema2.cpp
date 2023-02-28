
#include "Sema2.h"

#include <iostream>

using namespace ast;
using namespace sema;

Sema2::Sema2()
{
	for( auto& ty : types.types )
	{
		auto second = &ty.second;
		add_type_identifier(second);
	}
}

Type*
Sema2::CreateType(Type ty)
{
	return types.define_type(ty);
}

void
Sema2::push_scope()
{
	Name anonymous = Name("", lookup_.current().id());
	NameRef anonymous_ns = lookup_.add_name(anonymous);
	push_scope(anonymous_ns);
}

void
Sema2::push_scope(sema::NameRef nspace)
{
	lookup_.push_scope(nspace);
}

void
Sema2::pop_scope()
{
	lookup_.pop_scope();
}

sema::NameRef
Sema2::add_value_identifier(std::string const& name, TypeInstance type)
{
	return lookup_.add_name(Name::Var(name, lookup_.current().id(), type));
}

sema::NameRef
Sema2::add_type_identifier(Type const* type)
{
	return lookup_.add_name(
		Name::Type(type->get_name(), lookup_.current().id(), TypeInstance::OfType(type)));
}

NameLookupResult
Sema2::lookup_fqn(QualifiedName const& qname)
{
	return lookup_.lookup_fqn(qname);
}

NameLookupResult
Sema2::lookup_local(QualifiedName const& qname)
{
	return lookup_.lookup(qname);
}

std::optional<TypeInstance>
Sema2::get_expected_return()
{
	return current_scope->get_expected_return();
}

void
Sema2::set_expected_return(TypeInstance n)
{
	current_scope->set_expected_return(n);
}

void
Sema2::clear_expected_return()
{
	current_scope->clear_expected_return();
}

ir::IRModule*
Sema2::Module(AstNode* node, std::vector<ir::IRTopLevelStmt*> stmts)
{
	//
	auto mod = new ir::IRModule;

	mod->node = node;
	mod->stmts = stmts;

	return mod;
}

ir::IRTopLevelStmt*
Sema2::TLS(ir::IRExternFn* fn)
{
	auto nod = new ir::IRTopLevelStmt;

	nod->node = fn->node;
	nod->stmt.extern_fn = fn;
	nod->type = ir::IRTopLevelType::ExternFn;

	return nod;
}

ir::IRTopLevelStmt*
Sema2::TLS(ir::IRFunction* fn)
{
	auto nod = new ir::IRTopLevelStmt;

	nod->node = fn->node;
	nod->stmt.fn = fn;
	nod->type = ir::IRTopLevelType::Function;

	return nod;
}

ir::IRTopLevelStmt*
Sema2::TLS(ir::IRStruct* st)
{
	auto nod = new ir::IRTopLevelStmt;

	nod->node = st->node;
	nod->stmt.struct_decl = st;
	nod->type = ir::IRTopLevelType::Struct;

	return nod;
}

ir::IRTopLevelStmt*
Sema2::TLS(ir::IRUnion* st)
{
	auto nod = new ir::IRTopLevelStmt;

	nod->node = st->node;
	nod->stmt.union_decl = st;
	nod->type = ir::IRTopLevelType::Union;

	return nod;
}

ir::IRTopLevelStmt*
Sema2::TLS(ir::IREnum* st)
{
	auto nod = new ir::IRTopLevelStmt;

	nod->node = st->node;
	nod->stmt.enum_decl = st;
	nod->type = ir::IRTopLevelType::Enum;

	return nod;
}

ir::IRFunction*
Sema2::Fn(ast::AstNode* node, ir::IRProto* proto, ir::IRBlock* block)
{
	auto nod = new ir::IRFunction;

	nod->node = node;
	nod->proto = proto;
	nod->block = block;

	return nod;
}

ir::IRCall*
Sema2::FnCall(ast::AstNode* node, ir::IRExpr* call_target, ir::IRArgs* args)
{
	auto nod = new ir::IRCall;

	nod->node = node;
	nod->call_target = call_target;
	nod->args = args;

	return nod;
}

ir::IRExternFn*
Sema2::ExternFn(ast::AstNode* node, ir::IRProto* proto)
{
	auto nod = new ir::IRExternFn;

	nod->node = node;
	nod->proto = proto;

	return nod;
}

ir::IRProto*
Sema2::Proto(
	ast::AstNode* node,
	sema::NameRef name,
	std::vector<ir::ProtoArg> args,
	ir::IRTypeDeclaraor* rt,
	Type const* fn_type)
{
	auto nod = new ir::IRProto(node, name, args, rt, fn_type);

	return nod;
}

ir::IRBlock*
Sema2::Block(ast::AstNode* node, std::vector<ir::IRStmt*> stmts)
{
	auto nod = new ir::IRBlock;

	nod->node = node;
	nod->stmts = stmts;

	return nod;
}

ir::IRReturn*
Sema2::Return(ast::AstNode* node, ir::IRExpr* expr)
{
	auto nod = new ir::IRReturn;

	nod->node = node;
	nod->expr = expr;

	return nod;
}

ir::IRValueDecl*
Sema2::ValueDecl(ast::AstNode* node, std::string simple_name, ir::IRTypeDeclaraor* rt)
{
	auto nod = new ir::IRValueDecl(node, simple_name, rt);

	return nod;
}

ir::IRTypeDeclaraor*
Sema2::TypeDecl(ast::AstNode* node, sema::TypeInstance type)
{
	auto nod = new ir::IRTypeDeclaraor;

	nod->node = node;
	nod->type_instance = type;

	return nod;
}

ir::IRExpr*
Sema2::Expr(ir::IRCall* call)
{
	auto nod = new ir::IRExpr;

	nod->node = call->node;
	nod->expr.call = call;
	nod->type = ir::IRExprType::Call;

	auto t = call->call_target->type_instance.type->get_return_type();
	assert(t.has_value());
	nod->type_instance = t.value();

	return nod;
}

ir::IRExpr*
Sema2::Expr(ir::IRNumberLiteral* nl)
{
	auto nod = new ir::IRExpr;

	nod->node = nl->node;
	nod->expr.num_literal = nl;
	nod->type = ir::IRExprType::NumberLiteral;
	nod->type_instance = nl->type_instance;

	return nod;
}

ir::IRExpr*
Sema2::Expr(ir::IRStringLiteral* nl)
{
	auto nod = new ir::IRExpr;

	nod->node = nl->node;
	nod->expr.str_literal = nl;
	nod->type = ir::IRExprType::StringLiteral;
	nod->type_instance = nl->type_instance;

	return nod;
}

ir::IRExpr*
Sema2::Expr(ir::IRId* nl)
{
	auto nod = new ir::IRExpr;

	nod->node = nl->node;
	nod->expr.id = nl;
	nod->type = ir::IRExprType::Id;
	nod->type_instance = nl->type_instance;

	return nod;
}

ir::IRExpr*
Sema2::Expr(ir::IRValueDecl* nl)
{
	auto nod = new ir::IRExpr;

	nod->node = nl->node;
	nod->expr.decl = nl;
	nod->type = ir::IRExprType::ValueDecl;
	nod->type_instance = nl->type_decl->type_instance;

	return nod;
}

ir::IRExpr*
Sema2::Expr(ir::IRBinOp* nl)
{
	auto nod = new ir::IRExpr;

	nod->node = nl->node;
	nod->expr.binop = nl;
	nod->type = ir::IRExprType::BinOp;
	nod->type_instance = nl->type_instance;

	return nod;
}

ir::IRExpr*
Sema2::Expr(ir::IRMemberAccess* nl)
{
	auto nod = new ir::IRExpr;

	nod->node = nl->node;
	nod->expr.member_access = nl;
	nod->type = ir::IRExprType::MemberAccess;
	nod->type_instance = nl->member.type;

	return nod;
}

ir::IRExpr*
Sema2::Expr(ir::IRIndirectMemberAccess* nl)
{
	auto nod = new ir::IRExpr;

	nod->node = nl->node;
	nod->expr.indirect_member_access = nl;
	nod->type = ir::IRExprType::IndirectMemberAccess;
	nod->type_instance = nl->member.type;

	return nod;
}

ir::IRExpr*
Sema2::Expr(ir::IRAddressOf* nl)
{
	auto nod = new ir::IRExpr;

	nod->node = nl->node;
	nod->expr.addr_of = nl;
	nod->type = ir::IRExprType::AddressOf;
	nod->type_instance = nl->type_instance;

	return nod;
}

ir::IRExpr*
Sema2::Expr(ir::IRDeref* nl)
{
	auto nod = new ir::IRExpr;

	nod->node = nl->node;
	nod->expr.deref = nl;
	nod->type = ir::IRExprType::Deref;
	nod->type_instance = nl->type_instance;

	return nod;
}
ir::IRExpr*
Sema2::Expr(ir::IRArrayAccess* nl)
{
	auto nod = new ir::IRExpr;

	nod->node = nl->node;
	nod->expr.array_access = nl;
	nod->type = ir::IRExprType::ArrayAccess;
	nod->type_instance = nl->type_instance;

	return nod;
}

ir::IRExpr*
Sema2::Expr(ir::IREmpty* nl)
{
	auto nod = new ir::IRExpr;

	nod->node = nl->node;
	nod->expr.empty = nl;
	nod->type = ir::IRExprType::Empty;
	nod->type_instance = nl->type_instance;

	return nod;
}

ir::IRExpr*
Sema2::Expr(ir::IRIs* nl)
{
	auto nod = new ir::IRExpr;

	nod->node = nl->node;
	nod->expr.is = nl;
	nod->type = ir::IRExprType::Is;
	nod->type_instance = nl->type_instance;

	// TODO: Rewrite this so I don't have to manually pass up descriminations
	// through all possible expr nodes.
	// TODO: Also no business logic here
	if( nl->type_decl->type_instance.is_struct_type() )
		nod->discriminations.push_back(nl);

	return nod;
}

ir::IRExpr*
Sema2::Expr(ir::IRInitializer* nl)
{
	auto nod = new ir::IRExpr;

	nod->node = nl->node;
	nod->expr.initializer = nl;
	nod->type = ir::IRExprType::Initializer;
	nod->type_instance = nl->type_instance;

	return nod;
}

ir::IRStmt*
Sema2::Stmt(ir::IRReturn* ret)
{
	auto nod = new ir::IRStmt;

	nod->node = ret->node;
	nod->stmt.ret = ret;
	nod->type = ir::IRStmtType::Return;

	return nod;
}

ir::IRStmt*
Sema2::Stmt(ir::IRExpr* expr)
{
	auto nod = new ir::IRStmt;

	nod->node = expr->node;
	nod->stmt.expr = expr;
	nod->type = ir::IRStmtType::ExprStmt;

	return nod;
}

ir::IRStmt*
Sema2::Stmt(ir::IRLet* expr)
{
	auto nod = new ir::IRStmt;

	nod->node = expr->node;
	nod->stmt.let = expr;
	nod->type = ir::IRStmtType::Let;

	return nod;
}

ir::IRStmt*
Sema2::Stmt(ir::IRAssign* expr)
{
	auto nod = new ir::IRStmt;

	nod->node = expr->node;
	nod->stmt.assign = expr;
	nod->type = ir::IRStmtType::Assign;

	return nod;
}

ir::IRStmt*
Sema2::Stmt(ir::IRIf* e)
{
	auto nod = new ir::IRStmt;
	nod->node = e->node;
	nod->stmt.if_stmt = e;
	nod->type = ir::IRStmtType::If;
	return nod;
}

ir::IRStmt*
Sema2::Stmt(ir::IRFor* e)
{
	auto nod = new ir::IRStmt;
	nod->node = e->node;
	nod->stmt.for_stmt = e;
	nod->type = ir::IRStmtType::For;
	return nod;
}

ir::IRStmt*
Sema2::Stmt(ir::IRWhile* e)
{
	auto nod = new ir::IRStmt;
	nod->node = e->node;
	nod->stmt.while_stmt = e;
	nod->type = ir::IRStmtType::While;
	return nod;
}

ir::IRStmt*
Sema2::Stmt(ir::IRElse* e)
{
	auto nod = new ir::IRStmt;

	nod->node = e->node;
	nod->stmt.else_stmt = e;
	nod->type = ir::IRStmtType::Else;
	return nod;
}

ir::IRStmt*
Sema2::Stmt(ir::IRBlock* e)
{
	auto nod = new ir::IRStmt;

	nod->node = e->node;
	nod->stmt.block = e;
	nod->type = ir::IRStmtType::Block;
	return nod;
}

ir::IRStmt*
Sema2::Stmt(ir::IRSwitch* sw)
{
	auto nod = new ir::IRStmt;

	nod->node = sw->node;
	nod->stmt.switch_stmt = sw;
	nod->type = ir::IRStmtType::Switch;
	return nod;
}

ir::IRStmt*
Sema2::Stmt(ir::IRCase* c)
{
	auto nod = new ir::IRStmt;

	nod->node = c->node;
	nod->stmt.case_stmt = c;
	nod->type = ir::IRStmtType::Case;
	return nod;
}

ir::IRArgs*
Sema2::Args(ast::AstNode* node, std::vector<ir::IRExpr*> args)
{
	auto nod = new ir::IRArgs;

	nod->node = node;
	nod->args = args;

	return nod;
}

ir::IRNumberLiteral*
Sema2::NumberLiteral(ast::AstNode* node, sema::TypeInstance type_instance, long long val)
{
	auto nod = new ir::IRNumberLiteral;

	nod->node = node;
	nod->val = val;
	nod->type_instance = type_instance;

	return nod;
}

ir::IRStringLiteral*
Sema2::StringLiteral(ast::AstNode* node, sema::TypeInstance type_instance, std::string s)
{
	auto nod = new ir::IRStringLiteral;

	nod->node = node;
	nod->value = s;
	nod->type_instance = type_instance;

	return nod;
}

ir::IRId*
Sema2::Id(ast::AstNode* node, sema::NameRef name, sema::TypeInstance type, bool is_type_id)
{
	auto nod = new ir::IRId(node, name, type, is_type_id);

	return nod;
}

ir::IRSwitch*
Sema2::Switch(ast::AstNode* node, ir::IRExpr* expr, ir::IRBlock* block)
{
	//
	auto nod = new ir::IRSwitch;

	nod->node = node;
	nod->expr = expr;
	nod->block = block;

	return nod;
}

ir::IRCase*
Sema2::CaseDefault(ast::AstNode* node, ir::IRStmt* stmt)
{
	auto nod = new ir::IRCase;

	nod->node = node;
	nod->value = 0;
	nod->block = stmt;
	nod->is_default = true;

	return nod;
}

ir::IRCase*
Sema2::Case(ast::AstNode* node, long long expr, ir::IRStmt* stmt)
{
	//
	auto nod = new ir::IRCase;

	nod->node = node;
	nod->value = expr;
	nod->block = stmt;
	nod->is_default = false;

	return nod;
}

ir::IRCase*
Sema2::Case(ast::AstNode* node, long long expr, ir::IRStmt* stmt, std::vector<ir::IRParam*> args)
{
	//
	auto nod = new ir::IRCase;

	nod->node = node;
	nod->value = expr;
	nod->block = stmt;
	nod->discriminations = args;
	nod->is_default = false;

	return nod;
}

ir::IRLet*
Sema2::Let(ast::AstNode* node, sema::NameRef name, ir::IRAssign* assign)
{
	auto nod = new ir::IRLet(node, name, assign, assign->lhs->type_instance);

	return nod;
}

ir::IRIs*
Sema2::Is(
	ast::AstNode* node,
	ir::IRExpr* expr,
	ir::IRTypeDeclaraor* type_decl,
	sema::TypeInstance bool_type)
{
	auto nod = new ir::IRIs;

	nod->node = node;
	nod->type_decl = type_decl;
	nod->lhs = expr;
	nod->type_instance = bool_type;

	return nod;
}

ir::IRLet*
Sema2::LetEmpty(ast::AstNode* node, sema::NameRef name, sema::TypeInstance type)
{
	auto nod = new ir::IRLet(node, name, nullptr, type);

	return nod;
}

ir::IRIf*
Sema2::If(ast::AstNode* node, ir::IRExpr* bool_expr, ir::IRStmt* body, ir::IRElse* else_stmt)
{
	auto nod = new ir::IRIf;

	nod->node = node;
	nod->expr = bool_expr;
	nod->stmt = body;
	nod->else_stmt = else_stmt;

	return nod;
}

ir::IRIf*
Sema2::IfArrow(
	ast::AstNode* node,
	ir::IRExpr* cond,
	ir::IRStmt* block,
	ir::IRElse* else_stmt,
	std::vector<ir::IRParam*> args)
{
	auto nod = new ir::IRIf;

	nod->node = node;
	nod->expr = cond;
	nod->stmt = block;
	nod->else_stmt = else_stmt;
	nod->discriminations = args;

	return nod;
}

ir::IRElse*
Sema2::Else(ast::AstNode* node, ir::IRStmt* stmt)
{
	auto nod = new ir::IRElse;

	nod->node = node;
	nod->stmt = stmt;

	return nod;
}

ir::IRAssign*
Sema2::Assign(ast::AstNode* node, ast::AssignOp op, ir::IRExpr* lhs, ir::IRExpr* rhs)
{
	auto nod = new ir::IRAssign;

	nod->op = op;
	nod->node = node;
	nod->lhs = lhs;
	nod->rhs = rhs;

	return nod;
}

ir::IRBinOp*
Sema2::BinOp(
	ast::AstNode* node, ast::BinOp op, ir::IRExpr* lhs, ir::IRExpr* rhs, sema::TypeInstance type)
{
	auto nod = new ir::IRBinOp;

	nod->op = op;
	nod->node = node;
	nod->lhs = lhs;
	nod->rhs = rhs;
	nod->type_instance = type;

	return nod;
}

ir::IRStruct*
Sema2::Struct(
	ast::AstNode* node, sema::Type const* type, std::map<std::string, ir::IRValueDecl*> members)
{
	auto nod = new ir::IRStruct;

	nod->node = node;
	nod->members = members;
	nod->struct_type = type;

	return nod;
}

ir::IRUnion*
Sema2::Union(
	ast::AstNode* node, sema::Type const* type, std::map<std::string, ir::IRValueDecl*> members)
{
	auto nod = new ir::IRUnion;

	nod->node = node;
	nod->members = members;
	nod->union_type = type;

	return nod;
}

ir::IREnum*
Sema2::Enum(
	ast::AstNode* node, sema::Type const* type, std::map<std::string, ir::IREnumMember*> members)
{
	auto nod = new ir::IREnum;

	nod->node = node;
	// nod->members = members;
	nod->enum_type = type;

	return nod;
}

ir::IREnumMember*
Sema2::EnumMemberStruct(
	ast::AstNode* node,
	sema::Type const*,
	ir::IRStruct* struct_stmt,
	sema::NameRef name,
	EnumNominal idx)
{
	//
	auto nod = new ir::IREnumMember(node, idx, name, struct_stmt);

	return nod;
}

ir::IREnumMember*
Sema2::EnumMemberId(ast::AstNode* node, sema::Type const*, sema::NameRef name, EnumNominal idx)
{
	//
	auto nod = new ir::IREnumMember(node, idx, name);

	return nod;
}

ir::IRMemberAccess*
Sema2::MemberAccess(
	ast::AstNode* node, ir::IRExpr* expr, sema::MemberTypeInstance member, sema::NameRef name)
{
	auto nod = new ir::IRMemberAccess(node, name, member, expr);

	return nod;
}

ir::IRIndirectMemberAccess*
Sema2::IndirectMemberAccess(
	ast::AstNode* node, ir::IRExpr* expr, sema::MemberTypeInstance member, sema::NameRef name)
{
	auto nod = new ir::IRIndirectMemberAccess(node, name, member, expr);

	return nod;
}

ir::IRVarArg*
Sema2::VarArg(ast::AstNode* node)
{
	auto nod = new ir::IRVarArg;

	nod->node = node;

	return nod;
}

ir::IRAddressOf*
Sema2::AddressOf(ast::AstNode* node, ir::IRExpr* expr, sema::TypeInstance type)
{
	auto nod = new ir::IRAddressOf;

	nod->node = node;
	nod->expr = expr;
	nod->type_instance = type;

	return nod;
}

ir::IRDeref*
Sema2::Deref(ast::AstNode* node, ir::IRExpr* expr, sema::TypeInstance type)
{
	auto nod = new ir::IRDeref;

	nod->node = node;
	nod->expr = expr;
	nod->type_instance = type;

	return nod;
}

ir::IREmpty*
Sema2::Empty(ast::AstNode* node, sema::TypeInstance void_type)
{
	auto nod = new ir::IREmpty;

	nod->node = node;
	nod->type_instance = void_type;

	return nod;
}

ir::IRArrayAccess*
Sema2::ArrayAcess(
	ast::AstNode* node, ir::IRExpr* array_target, ir::IRExpr* expr, sema::TypeInstance type)
{
	auto nod = new ir::IRArrayAccess;

	nod->node = node;
	nod->expr = expr;
	nod->array_target = array_target;
	nod->type_instance = type;

	return nod;
}

ir::IRParam*
Sema2::IRParam(ast::AstNode* node, ir::IRValueDecl* decl)
{
	auto nod = new ir::IRParam;

	nod->node = node;
	nod->data.value_decl = decl;
	nod->type = ir::IRParamType::ValueDecl;

	return nod;
}

ir::IRParam*
Sema2::IRParam(ast::AstNode* node, ir::IRVarArg* decl)
{
	auto nod = new ir::IRParam;

	nod->node = node;
	nod->data.var_arg = decl;
	nod->type = ir::IRParamType::VarArg;

	return nod;
}

ir::IRFor*
Sema2::For(
	ast::AstNode* node, ir::IRExpr* condition, ir::IRStmt* init, ir::IRStmt* end, ir::IRStmt* body)
{
	auto nod = new ir::IRFor;

	nod->node = node;
	nod->condition = condition;
	nod->init = init;
	nod->end = end;
	nod->body = body;

	return nod;
}
ir::IRWhile*
Sema2::While(ast::AstNode* node, ir::IRExpr* condition, ir::IRStmt* body)
{
	auto nod = new ir::IRWhile;

	nod->node = node;
	nod->condition = condition;
	nod->body = body;

	return nod;
}

ir::IRInitializer*
Sema2::Initializer(
	ast::AstNode* node,
	sema::NameRef name,
	std::vector<ir::IRDesignator*> initializers,
	sema::TypeInstance type_instance)
{
	//
	auto nod = new ir::IRInitializer(node, name, initializers, type_instance);

	return nod;
}

ir::IRDesignator*
Sema2::Designator(ast::AstNode* node, sema::MemberTypeInstance member, ir::IRExpr* expr)
{
	auto nod = new ir::IRDesignator;

	nod->node = node;
	nod->member = member;
	nod->expr = expr;

	return nod;
}