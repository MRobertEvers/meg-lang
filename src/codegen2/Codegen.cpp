#include "Codegen.h"

#include "Codegen/CGNotImpl.h"
#include "Codegen/codegen_call.h"
#include "Codegen/codegen_function.h"
#include "Codegen/codegen_return.h"
#include "Codegen/lookup.h"
#include "ast2/AstCasts.h"

using namespace cg;
using namespace ast;

static void
establish_llvm_builtin_types(
	CG& cg, sema::Types& types, std::map<sema::Type const*, llvm::Type*>& lut)
{
	lut.emplace(types.u8_type(), llvm::Type::getInt8Ty(*cg.Context));
	lut.emplace(types.u16_type(), llvm::Type::getInt16Ty(*cg.Context));
	lut.emplace(types.u32_type(), llvm::Type::getInt32Ty(*cg.Context));
	lut.emplace(types.i8_type(), llvm::Type::getInt8Ty(*cg.Context));
	lut.emplace(types.i16_type(), llvm::Type::getInt16Ty(*cg.Context));
	lut.emplace(types.i32_type(), llvm::Type::getInt32Ty(*cg.Context));
	lut.emplace(types.void_type(), llvm::Type::getVoidTy(*cg.Context));
}

static llvm::Value*
promote_to_value(CG& cg, llvm::Value* Val)
{
	// TODO: Opaque pointers, don't rel
	auto PointedToType = Val->getType()->getPointerElementType();
	return cg.Builder->CreateLoad(PointedToType, Val);
}

static llvm::Value*
__deprecate_promote_to_value(CG& cg, llvm::Value* Val)
{
	// TODO: Opaque pointers, don't rel
	if( Val->getType()->isPointerTy() )
		return promote_to_value(cg, Val);
	else
		return Val;
}

CG::CG(sema::Sema2& sema)
	: sema(sema)
{
	Context = std::make_unique<llvm::LLVMContext>();
	Module = std::make_unique<llvm::Module>("this_module", *Context);
	// Create a new builder for the module.
	Builder = std::make_unique<llvm::IRBuilder<>>(*Context);
	// TODO: Populate builtin types that automatically map the llvm types.

	establish_llvm_builtin_types(*this, sema.types, this->types);
}

void
CG::add_function(String const& name, LLVMFnSigInfo context)
{
	Functions.emplace(name, context);
	types.emplace(context.sema_fn_ty, context.llvm_fn_ty);
	values.emplace(name, context.llvm_fn);
}

// Scope*
// CG::push_scope()
// {
// 	auto s = Scope(current_scope);
// 	scopes.push_back(s);
// 	current_scope = &scopes.back();

// 	return current_scope;
// }

// void
// CG::pop_scope()
// {
// 	assert(current_scope->get_parent());
// 	current_scope = current_scope->get_parent();
// }

CGResult<CGExpr>
CG::codegen_module(ir::IRModule* mod)
{
	for( auto tls : *mod->stmts )
	{
		auto tlsr = codegen_tls(tls);
		if( !tlsr.ok() )
			return tlsr;
	}

	return CGExpr();
}

CGResult<CGExpr>
CG::codegen_tls(ir::IRTopLevelStmt* tls)
{
	switch( tls->type )
	{
	case ir::IRTopLevelType::ExternFn:
		return codegen_extern_fn(tls->stmt.extern_fn);
	case ir::IRTopLevelType::Function:
		return codegen_function(*this, tls->stmt.fn);
	case ir::IRTopLevelType::Struct:
		return codegen_struct(tls->stmt.struct_decl);
	}

	return NotImpl();
}

CGResult<CGExpr>
CG::codegen_stmt(cg::LLVMFnInfo& fn, ir::IRStmt* stmt)
{
	switch( stmt->type )
	{
	case ir::IRStmtType::ExprStmt:
		return codegen_expr(fn, stmt->stmt.expr);
	case ir::IRStmtType::Return:
		return codegen_return(*this, fn, stmt->stmt.ret);
	case ir::IRStmtType::Assign:
		return codegen_assign(fn, stmt->stmt.assign);
	case ir::IRStmtType::Let:
		return codegen_let(fn, stmt->stmt.let);
	case ir::IRStmtType::If:
		return codegen_if(fn, stmt->stmt.if_stmt);
	case ir::IRStmtType::Else:
		return codegen_else(fn, stmt->stmt.else_stmt);
	case ir::IRStmtType::Block:
		return codegen_block(fn, stmt->stmt.block);
	}

	return NotImpl();
}

CGResult<CGExpr>
CG::codegen_expr(cg::LLVMFnInfo& fn, ir::IRExpr* expr)
{
	return codegen_expr(fn, expr, std::optional<LValue>());
}

CGResult<CGExpr>
CG::codegen_expr(cg::LLVMFnInfo& fn, ir::IRExpr* expr, std::optional<LValue> lvalue)
{
	switch( expr->type )
	{
	case ir::IRExprType::Call:
		return codegen_call(*this, fn, expr->expr.call, lvalue);
	case ir::IRExprType::Id:
		return codegen_id(expr->expr.id);
	case ir::IRExprType::NumberLiteral:
		return codegen_number_literal(expr->expr.num_literal);
	case ir::IRExprType::StringLiteral:
		return codegen_string_literal(expr->expr.str_literal);
	case ir::IRExprType::ValueDecl:
		return codegen_value_decl(expr->expr.decl);
	case ir::IRExprType::BinOp:
		return codegen_binop(fn, expr->expr.binop);
	case ir::IRExprType::MemberAccess:
		return codegen_member_access(fn, expr->expr.member_access);
	}

	return NotImpl();
}

CGResult<CGExpr>
CG::codegen_extern_fn(ir::IRExternFn* extern_fn)
{
	return codegen_function_proto(*this, extern_fn->proto);
}

CGResult<CGExpr>
CG::codegen_member_access(cg::LLVMFnInfo& fn, ir::IRMemberAccess* ma)
{
	auto exprr = codegen_expr(fn, ma->expr);
	if( !exprr.ok() )
		return exprr;
	auto expr = exprr.unwrap();
	auto ExprValue = expr.as_value();

	auto expr_ty = ma->expr->type_instance;
	assert(expr_ty.type->is_struct_type() && expr_ty.indirection_level == 0);
	auto llvm_expr_tyr = get_type(*this, expr_ty);
	if( !llvm_expr_tyr.ok() )
		return llvm_expr_tyr;
	auto ExprTy = llvm_expr_tyr.unwrap();

	auto struct_ty_name = expr_ty.type->get_name();

	auto member_name = *ma->member_name;
	auto maybe_member = expr_ty.type->get_member(member_name);
	assert(maybe_member.has_value());

	auto member = maybe_member.value();
	auto llvm_member_tyr = get_type(*this, member.type);
	if( !llvm_member_tyr.ok() )
		return llvm_member_tyr;
	auto MemberTy = llvm_member_tyr.unwrap();

	auto MemberPtr =
		Builder->CreateStructGEP(ExprTy, ExprValue, member.idx, struct_ty_name + "." + member_name);

	return CGExpr(MemberPtr);
}

CGResult<CGExpr>
CG::codegen_let(cg::LLVMFnInfo& fn, ir::IRLet* let)
{
	auto name = let->name;
	auto type = let->assign->rhs->type_instance;
	auto typer = get_type(*this, type);
	if( !typer.ok() )
		return typer;
	auto Type = typer.unwrap();

	llvm::AllocaInst* Alloca = Builder->CreateAlloca(Type, nullptr, *name);
	values.emplace(*name, Alloca);

	auto assignr = codegen_assign(fn, let->assign);
	if( !assignr.ok() )
		return assignr;

	return CGExpr(Alloca);
}

// CGResult<CGExpr>
// CG::codegen_assign(ir::IRAssign* assign)
// {
// 	return codegen_assign(assign, std::optional<CGExpr>());
// }

CGResult<CGExpr>
CG::codegen_assign(cg::LLVMFnInfo& fn, ir::IRAssign* assign)
{
	auto lhsr = codegen_expr(fn, assign->lhs);
	if( !lhsr.ok() )
		return lhsr;

	LValue __temp_replace = LValue(lhsr.unwrap().as_value(), lhsr.unwrap().as_value()->getType());

	auto lexpr = lhsr.unwrap();
	auto rhsr = codegen_expr(fn, assign->rhs, __temp_replace);
	if( !rhsr.ok() )
		return rhsr;

	auto rexpr = rhsr.unwrap();

	// TODO: The RHS might be a Struct type name.
	// E.g.
	//
	// let my_point = Point;
	//
	// In this case, assignment is a no op.
	// TODO: Later, insert a constructor call?
	if( rexpr.type == CGExprType::Empty )
		return CGExpr();

	// TODO: lvalue rvalue?
	auto LValue = lexpr.as_value();
	auto RValue = rexpr.as_value();

	assert(LValue && RValue && "nullptr for assignment!");

	// TODO: Promote to value really only needs to be done for allocas??
	// Confusing, we'll see where this goes.
	switch( assign->op )
	{
	case ast::AssignOp::add:
	{
		auto LValuePromoted = promote_to_value(*this, LValue);
		auto TempRValue = Builder->CreateAdd(RValue, LValuePromoted);
		Builder->CreateStore(TempRValue, LValue);
	}
	break;
	case ast::AssignOp::sub:
	{
		auto LValuePromoted = promote_to_value(*this, LValue);
		auto TempRValue = Builder->CreateSub(LValuePromoted, RValue);
		Builder->CreateStore(TempRValue, LValue);
	}
	break;
	case ast::AssignOp::mul:
	{
		auto LValuePromoted = promote_to_value(*this, LValue);
		auto TempRValue = Builder->CreateMul(RValue, LValuePromoted);
		Builder->CreateStore(TempRValue, LValue);
	}
	break;
	case ast::AssignOp::div:
	{
		auto LValuePromoted = promote_to_value(*this, LValue);
		auto TempRValue = Builder->CreateSDiv(RValue, LValuePromoted);
		Builder->CreateStore(TempRValue, LValue);
	}
	break;
	case ast::AssignOp::assign:
		// TODO: Constant expressions return their exact values,
		// LValue expressions return a pointer to them, so LValues need
		// to be promoted. Need to create LValue type
		auto RValuePromoted = __deprecate_promote_to_value(*this, RValue);
		Builder->CreateStore(RValuePromoted, LValue);
		break;
	}

	return lhsr.unwrap();
}

CGResult<CGExpr>
CG::codegen_number_literal(ir::IRNumberLiteral* lit)
{
	//
	llvm::Value* ConstInt = llvm::ConstantInt::get(*Context, llvm::APInt(32, lit->val, true));

	return CGExpr(ConstInt, true);
}

CGResult<CGExpr>
CG::codegen_string_literal(ir::IRStringLiteral* lit)
{
	//
	auto Literal = llvm::ConstantDataArray::getString(*Context, lit->value->c_str(), true);

	llvm::GlobalVariable* GVStr = new llvm::GlobalVariable(
		*Module, Literal->getType(), true, llvm::GlobalValue::InternalLinkage, Literal);
	llvm::Constant* zero = llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(*Context));
	llvm::Constant* indices[] = {zero, zero};
	llvm::Constant* StrVal =
		llvm::ConstantExpr::getGetElementPtr(Literal->getType(), GVStr, indices);

	return CGExpr(StrVal, true);
}

CGResult<CGExpr>
CG::codegen_value_decl(ir::IRValueDecl* decl)
{
	auto valuer = get_value(*this, *decl->name);
	assert(valuer.has_value());

	return valuer.value();
}

CGResult<CGExpr>
CG::codegen_binop(cg::LLVMFnInfo& fn, ir::IRBinOp* binop)
{
	auto lhsr = codegen_expr(fn, binop->lhs);
	if( !lhsr.ok() )
		return lhsr;

	auto rhsr = codegen_expr(fn, binop->rhs);
	if( !rhsr.ok() )
		return rhsr;

	auto lexpr = lhsr.unwrap();
	auto rexpr = rhsr.unwrap();
	// TODO: lvalue rvalue?
	auto LValue = lexpr.as_value();
	auto RValue = rexpr.as_value();

	assert(LValue && RValue && "nullptr for assignment!");

	// TODO: If the values are imediate then we don't need to promote.
	// TODO: If both values are constant, just do the computation.
	auto L = __deprecate_promote_to_value(*this, LValue); // promote_to_value(*this, LValue);
	auto R = __deprecate_promote_to_value(*this, RValue); // promote_to_value(*this, RValue);

	assert(L->getType()->isIntegerTy() && R->getType()->isIntegerTy());

	auto Op = binop->op;
	switch( Op )
	{
	case BinOp::plus:
		return CGExpr(Builder->CreateAdd(L, R, "addtmp"));
	case BinOp::minus:
		return CGExpr(Builder->CreateSub(L, R, "subtmp"));
	case BinOp::star:
		return CGExpr(Builder->CreateMul(L, R, "multmp"));
	case BinOp::slash:
		return CGExpr(Builder->CreateSDiv(L, R, "divtmp"));
	case BinOp::gt:
		return CGExpr(Builder->CreateICmpUGT(L, R, "cmptmp"));
	case BinOp::gte:
		return CGExpr(Builder->CreateICmpUGE(L, R, "cmptmp"));
	case BinOp::lt:
		return CGExpr(Builder->CreateICmpULT(L, R, "cmptmp"));
	case BinOp::lte:
		return CGExpr(Builder->CreateICmpULE(L, R, "cmptmp"));
	case BinOp::cmp:
		return CGExpr(Builder->CreateICmpEQ(L, R, "cmptmp"));
	case BinOp::ne:
		return CGExpr(Builder->CreateICmpNE(L, R, "cmptmp"));
	case BinOp::and_lex:
		return CGExpr(Builder->CreateAnd(L, R, "cmptmp"));
	case BinOp::or_lex:
		return CGExpr(Builder->CreateOr(L, R, "cmptmp"));
	default:
		return NotImpl();
	}
}

CGResult<CGExpr>
CG::codegen_id(ir::IRId* id)
{
	// auto iter_type = types.find(id->type_instance.type);
	auto value = get_value(*this, *id->name);
	if( value.has_value() )
		return value.value();

	// TODO: This supports 'let my_point = Point' initialization.
	// I can see the "Codegen Id" function getting a little wild.
	// Need to rethink it.
	// TODO: This should be get type by name...
	// Otherwise bad codegen.
	auto maybe_type = get_type(*this, id->type_instance);
	if( maybe_type.ok() )
		return CGExpr();

	return CGError("Undeclared identifier! " + *id->name);
}

CGResult<CGExpr>
CG::codegen_if(cg::LLVMFnInfo& fn, ir::IRIf* ir_if)
{
	//
	auto exprr = codegen_expr(fn, ir_if->expr);
	if( !exprr.ok() )
		return exprr;
	auto cond_expr = exprr.unwrap();
	auto llvm_cond_v = cond_expr.as_value();

	auto llvm_fn = fn.sig_info.llvm_fn;

	// Create blocks for the then and else cases.  Insert the 'then' block at the
	// end of the function.
	llvm::BasicBlock* llvm_then_bb = llvm::BasicBlock::Create(*Context, "then", llvm_fn);
	llvm::BasicBlock* llvm_else_bb = llvm::BasicBlock::Create(*Context, "else");
	llvm::BasicBlock* llvm_merge_bb = llvm::BasicBlock::Create(*Context);

	Builder->CreateCondBr(llvm_cond_v, llvm_then_bb, llvm_else_bb);

	// Emit then value.
	Builder->SetInsertPoint(llvm_then_bb);

	auto then_stmtr = codegen_stmt(fn, ir_if->stmt);
	if( !then_stmtr.ok() )
		return then_stmtr;

	Builder->CreateBr(llvm_else_bb);
	llvm_fn->getBasicBlockList().push_back(llvm_else_bb);
	Builder->SetInsertPoint(llvm_else_bb);

	if( ir_if->else_stmt != nullptr )
	{
		auto then_stmtr = codegen_stmt(fn, ir_if->stmt);
		if( !then_stmtr.ok() )
			return then_stmtr;
	}
	Builder->CreateBr(llvm_merge_bb);

	// Codegen of 'Else' can change the current block, update ElseBB for the PHI.
	llvm_else_bb = Builder->GetInsertBlock();

	// Emit merge block.
	llvm_fn->getBasicBlockList().push_back(llvm_merge_bb);
	Builder->SetInsertPoint(llvm_merge_bb);

	return CGExpr();
}

CGResult<CGExpr>
CG::codegen_else(cg::LLVMFnInfo& fn, ir::IRElse* ir_else)
{
	//
	return codegen_stmt(fn, ir_else->stmt);
}

CGResult<CGExpr>
CG::codegen_block(cg::LLVMFnInfo& fn, ir::IRBlock* ir_block)
{
	//
	for( auto stmt : *ir_block->stmts )
	{
		//
		auto stmtr = codegen_stmt(fn, stmt);
		if( !stmtr.ok() )
			return stmtr;
	}

	return CGExpr();
}

CGResult<CGExpr>
CG::codegen_struct(ir::IRStruct* st)
{
	Vec<llvm::Type*> members;
	for( auto& member : *st->members )
	{
		auto value_decl = member.second;
		auto typerr = get_type(*this, value_decl->type_decl);
		if( !typerr.ok() )
			return typerr;

		members.push_back(typerr.unwrap());
	}

	auto struct_type = st->struct_type;
	auto name = struct_type->get_name();
	llvm::StructType* StructTy = llvm::StructType::create(*Context, members, name);

	this->types.emplace(struct_type, StructTy);

	return CGExpr();
}

std::optional<llvm::Type*>
CG::find_type(sema::Type const* ty)
{
	auto t = types.find(ty);
	if( t != types.end() )
		return t->second;
	else
		return std::optional<llvm::Type*>();
}