#include "Codegen.h"

#include "Codegen/CGNotImpl.h"
#include "Codegen/RValue.h"
#include "Codegen/codegen_addressof.h"
#include "Codegen/codegen_array_access.h"
#include "Codegen/codegen_assign.h"
#include "Codegen/codegen_call.h"
#include "Codegen/codegen_deref.h"
#include "Codegen/codegen_function.h"
#include "Codegen/codegen_initializer.h"
#include "Codegen/codegen_is.h"
#include "Codegen/codegen_member_access.h"
#include "Codegen/codegen_return.h"
#include "Codegen/codegen_string_literal.h"
#include "Codegen/codegen_while.h"
#include "Codegen/lookup.h"
#include "Codegen/operand.h"
#include "ast2/AstCasts.h"

using namespace cg;
using namespace ast;

static String
to_single_name(Vec<String*>* list)
{
	auto name = String();

	bool first = true;
	for( auto part : *list )
	{
		if( !first )
			name += "#";
		name += *part;

		first = false;
	}

	return name;
}

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

	auto lvalue = LValue(context.llvm_fn, context.llvm_fn_ty);
	values.emplace(name, lvalue);
}

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
	case ir::IRTopLevelType::Union:
		return codegen_union(tls->stmt.union_decl);
	case ir::IRTopLevelType::Enum:
		return codegen_enum(tls->stmt.enum_decl);
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
		return codegen_assign(*this, fn, stmt->stmt.assign);
	case ir::IRStmtType::Let:
		return codegen_let(fn, stmt->stmt.let);
	case ir::IRStmtType::If:
		return codegen_if(fn, stmt->stmt.if_stmt);
	case ir::IRStmtType::For:
		return codegen_for(fn, stmt->stmt.for_stmt);
	case ir::IRStmtType::While:
		return codegen_while(*this, fn, stmt->stmt.while_stmt);
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
	case ir::IRExprType::ArrayAccess:
		return codegen_array_access(*this, fn, expr->expr.array_access);
	case ir::IRExprType::Id:
		return codegen_id(expr->expr.id);
	case ir::IRExprType::NumberLiteral:
		return codegen_number_literal(expr->expr.num_literal);
	case ir::IRExprType::StringLiteral:
		return codegen_string_literal(*this, expr->expr.str_literal);
	case ir::IRExprType::ValueDecl:
		return codegen_value_decl(expr->expr.decl);
	case ir::IRExprType::BinOp:
		return codegen_binop(fn, expr->expr.binop);
	case ir::IRExprType::MemberAccess:
		return codegen_member_access(*this, fn, expr->expr.member_access);
	case ir::IRExprType::IndirectMemberAccess:
		return codegen_indirect_member_access(*this, fn, expr->expr.indirect_member_access);
	case ir::IRExprType::AddressOf:
		return codegen_addressof(*this, fn, expr->expr.addr_of);
	case ir::IRExprType::Deref:
		return codegen_deref(*this, fn, expr->expr.deref);
	case ir::IRExprType::Is:
		return codegen_is(*this, fn, expr->expr.is);
	case ir::IRExprType::Initializer:
		return codegen_initializer(*this, fn, expr->expr.initializer, lvalue);
	case ir::IRExprType::Empty:
		return CGExpr();
	}

	return NotImpl();
}

CGResult<CGExpr>
CG::codegen_extern_fn(ir::IRExternFn* extern_fn)
{
	return codegen_function_proto(*this, extern_fn->proto);
}

CGResult<CGExpr>
CG::codegen_let(cg::LLVMFnInfo& fn, ir::IRLet* ir_let)
{
	auto name = ir_let->name;
	auto type = ir_let->type_instance;

	auto storage_type = type.storage_type();

	auto typer = get_type(*this, storage_type);
	if( !typer.ok() )
		return typer;
	auto llvm_allocated_type = typer.unwrap();

	llvm::AllocaInst* llvm_alloca = Builder->CreateAlloca(llvm_allocated_type, nullptr, *name);
	auto lvalue = LValue(llvm_alloca, llvm_allocated_type);
	values.insert_or_assign(*name, lvalue);

	if( !ir_let->is_empty() )
	{
		auto assignr = codegen_assign(*this, fn, ir_let->assign);
		if( !assignr.ok() )
			return assignr;
	}

	return CGExpr();
}

CGResult<CGExpr>
CG::codegen_number_literal(ir::IRNumberLiteral* lit)
{
	//
	llvm::Value* llvm_const_int = llvm::ConstantInt::get(*Context, llvm::APInt(32, lit->val, true));

	return CGExpr::MakeRValue(RValue(llvm_const_int, llvm_const_int->getType()));
}

CGResult<CGExpr>
CG::codegen_value_decl(ir::IRValueDecl* decl)
{
	auto valuer = get_value(*this, *decl->name);
	assert(valuer.has_value());

	return CGExpr::MakeAddress(valuer.value().address());
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

	auto llvm_lhs = codegen_operand_expr(*this, lexpr);
	auto llvm_rhs = codegen_operand_expr(*this, rexpr);

	assert(llvm_lhs && llvm_rhs && "nullptr for assignment!");

	assert(llvm_lhs->getType()->isIntegerTy() && llvm_rhs->getType()->isIntegerTy());

	auto Op = binop->op;
	switch( Op )
	{
	case BinOp::plus:
		return CGExpr::MakeRValue(RValue(Builder->CreateAdd(llvm_lhs, llvm_rhs)));
	case BinOp::minus:
		return CGExpr::MakeRValue(RValue(Builder->CreateSub(llvm_lhs, llvm_rhs)));
	case BinOp::star:
		return CGExpr::MakeRValue(RValue(Builder->CreateMul(llvm_lhs, llvm_rhs)));
	case BinOp::slash:
		return CGExpr::MakeRValue(RValue(Builder->CreateSDiv(llvm_lhs, llvm_rhs)));
	case BinOp::gt:
		return CGExpr::MakeRValue(RValue(Builder->CreateICmpSGT(llvm_lhs, llvm_rhs)));
	case BinOp::gte:
		return CGExpr::MakeRValue(RValue(Builder->CreateICmpSGE(llvm_lhs, llvm_rhs)));
	case BinOp::lt:
		return CGExpr::MakeRValue(RValue(Builder->CreateICmpSLT(llvm_lhs, llvm_rhs)));
	case BinOp::lte:
		return CGExpr::MakeRValue(RValue(Builder->CreateICmpSLE(llvm_lhs, llvm_rhs)));
	case BinOp::cmp:
		return CGExpr::MakeRValue(RValue(Builder->CreateICmpEQ(llvm_lhs, llvm_rhs)));
	case BinOp::ne:
		return CGExpr::MakeRValue(RValue(Builder->CreateICmpNE(llvm_lhs, llvm_rhs)));
	case BinOp::and_op:
		return CGExpr::MakeRValue(RValue(Builder->CreateAnd(llvm_lhs, llvm_rhs)));
	case BinOp::or_op:
		return CGExpr::MakeRValue(RValue(Builder->CreateOr(llvm_lhs, llvm_rhs)));
	default:
		return NotImpl();
	}
}

CGResult<CGExpr>
CG::codegen_id(ir::IRId* id)
{
	// auto iter_type = types.find(id->type_instance.type);
	auto value = get_value(*this, to_single_name(id->name));
	if( value.has_value() )
		return CGExpr::MakeAddress(value.value().address());

	// TODO: This supports 'let my_point = Point' initialization.
	// I can see the "Codegen Id" function getting a little wild.
	// Need to rethink it.
	// TODO: This should be get type by name...
	// Otherwise bad codegen.
	auto maybe_type = get_type(*this, id->type_instance);
	if( maybe_type.ok() )
		return CGExpr();

	return CGError("Undeclared identifier! " + to_single_name(id->name));
}

CGResult<CGExpr>
CG::codegen_if(cg::LLVMFnInfo& fn, ir::IRIf* ir_if)
{
	//
	auto exprr = codegen_expr(fn, ir_if->expr);
	if( !exprr.ok() )
		return exprr;
	auto cond_expr = exprr.unwrap();
	auto llvm_cond_v = codegen_operand_expr(*this, cond_expr);

	auto llvm_fn = fn.sig_info.llvm_fn;

	// Create blocks for the then and else cases.  Insert the 'then' block at the
	// end of the function.
	llvm::BasicBlock* llvm_then_bb = llvm::BasicBlock::Create(*Context, "then", llvm_fn);
	llvm::BasicBlock* llvm_else_bb = llvm::BasicBlock::Create(*Context, "else");

	// bool own_merge_block = false;
	// if( !fn.merge_block().has_value() )
	// {
	// 	fn.set_merge_block(llvm::BasicBlock::Create(*Context));
	// 	own_merge_block = true;
	// }

	// llvm::BasicBlock* llvm_merge_bb = fn.merge_block().value();
	llvm::BasicBlock* llvm_merge_bb = llvm::BasicBlock::Create(*Context);

	Builder->CreateCondBr(llvm_cond_v, llvm_then_bb, llvm_else_bb);

	// Emit then value.
	Builder->SetInsertPoint(llvm_then_bb);

	// Inject any discriminations
	if( ir_if->discriminations )
	{
		int ind = 0;
		for( auto param : *ir_if->discriminations )
		{
			auto ir_value_decl = param->data.value_decl;
			auto ir_type_decl = param->data.value_decl->type_decl;
			auto llvm_type = get_type(*this, ir_type_decl).unwrap();

			auto name = ir_value_decl->name;

			auto enum_value = cond_expr.get_discrimination(ind).address();
			auto llvm_enum_value = enum_value.llvm_pointer();
			auto llvm_enum_type = enum_value.llvm_allocated_type();
			auto llvm_member_value_ptr =
				Builder->CreateStructGEP(llvm_enum_type, llvm_enum_value, 1);

			auto llvm_member_value =
				Builder->CreateBitCast(llvm_member_value_ptr, llvm_type->getPointerTo());

			auto lval = LValue(llvm_member_value, llvm_type);
			this->values.insert_or_assign(*name, lval);

			ind++;
		}
	}

	auto then_stmtr = codegen_stmt(fn, ir_if->stmt);
	if( !then_stmtr.ok() )
		return then_stmtr;

	Builder->CreateBr(llvm_merge_bb);
	llvm_fn->getBasicBlockList().push_back(llvm_else_bb);
	Builder->SetInsertPoint(llvm_else_bb);

	if( ir_if->else_stmt != nullptr )
	{
		auto then_stmtr = codegen_else(fn, ir_if->else_stmt);
		if( !then_stmtr.ok() )
			return then_stmtr;
	}

	// Emit merge block.
	// if( own_merge_block )
	// {
	Builder->CreateBr(llvm_merge_bb);
	llvm_fn->getBasicBlockList().push_back(llvm_merge_bb);
	Builder->SetInsertPoint(llvm_merge_bb);
	// fn.clear_merge_block();
	// }

	return CGExpr();
}

CGResult<CGExpr>
CG::codegen_for(cg::LLVMFnInfo& fn, ir::IRFor* ir_for)
{
	auto forr = codegen_stmt(fn, ir_for->init);
	if( !forr.ok() )
		return forr;
	auto for_stmt = forr.unwrap();

	auto llvm_fn = fn.llvm_fn();
	llvm::BasicBlock* llvm_cond_bb = llvm::BasicBlock::Create(*Context, "condition", llvm_fn);
	llvm::BasicBlock* llvm_loop_bb = llvm::BasicBlock::Create(*Context, "loop");
	llvm::BasicBlock* llvm_done_bb = llvm::BasicBlock::Create(*Context, "after_loop");

	// There are no implicit fallthroughs, we have to make an
	// explicit fallthrough from the current block to the LoopStartBB
	Builder->CreateBr(llvm_cond_bb);
	Builder->SetInsertPoint(llvm_cond_bb);

	auto condr = codegen_expr(fn, ir_for->condition);
	if( !condr.ok() )
		return condr;
	auto cond = condr.unwrap();

	auto llvm_cond = codegen_operand_expr(*this, cond);
	// TODO: Typecheck is boolean or cast?
	Builder->CreateCondBr(llvm_cond, llvm_loop_bb, llvm_done_bb);
	llvm_fn->getBasicBlockList().push_back(llvm_loop_bb);
	Builder->SetInsertPoint(llvm_loop_bb);

	auto bodyr = codegen_stmt(fn, ir_for->body);
	if( !bodyr.ok() )
		return bodyr;
	auto body = bodyr.unwrap();

	auto endr = codegen_stmt(fn, ir_for->end);
	if( !endr.ok() )
		return endr;
	auto end = endr.unwrap();

	Builder->CreateBr(llvm_cond_bb);

	llvm_fn->getBasicBlockList().push_back(llvm_done_bb);
	Builder->SetInsertPoint(llvm_done_bb);

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
	auto previous_scope = this->values;
	for( auto stmt : *ir_block->stmts )
	{
		//
		auto stmtr = codegen_stmt(fn, stmt);
		if( !stmtr.ok() )
			return stmtr;
	}
	this->values = previous_scope;

	return CGExpr();
}

CGResult<CGExpr>
CG::codegen_struct(ir::IRStruct* st)
{
	Vec<llvm::Type*> members;
	for( int i = 0; i < st->struct_type->get_member_count(); i++ )
	{
		// Order in llvm type must match the order in the sema type
		// because the member idx is used to access members
		// as an index.
		auto member = st->struct_type->get_member(i);

		auto typerr = get_type(*this, member.type);
		if( !typerr.ok() )
			return typerr;

		members.push_back(typerr.unwrap());
	}

	auto struct_type = st->struct_type;
	auto name = struct_type->get_name();
	llvm::StructType* llvm_struct_type = llvm::StructType::create(*Context, members, name);

	this->types.emplace(struct_type, llvm_struct_type);

	return CGExpr();
}

CGResult<CGExpr>
CG::codegen_union(ir::IRUnion* st)
{
	llvm::Type* max_type_by_size = nullptr;
	int max_size = 0;
	for( auto& member : *st->members )
	{
		auto value_decl = member.second;
		auto typerr = get_type(*this, value_decl->type_decl);
		if( !typerr.ok() )
			return typerr;

		auto llvm_type = typerr.unwrap();

		auto llvm_size = Module->getDataLayout().getTypeAllocSize(llvm_type);
		if( llvm_size.getKnownMinSize() > max_size )
		{
			max_type_by_size = llvm_type;
			max_size = llvm_size.getKnownMinSize();
		}
	}

	Vec<llvm::Type*> members = {max_type_by_size};

	auto union_type = st->union_type;
	auto name = union_type->get_name();
	llvm::StructType* llvm_union_type = llvm::StructType::create(*Context, members, name);

	this->types.emplace(union_type, llvm_union_type);

	return CGExpr();
}

/**
 * @brief
 *  For enums, need to look up name,
 * and have a mapping of member to llvm type
 *
 * Defines a struct, and a union
 *
 * struct MyEnum {
 * 	int type;
 * 	union {
 * 		...
 * 	}
 * }
 * @param st
 * @return CGResult<CGExpr>
 */
CGResult<CGExpr>
CG::codegen_enum(ir::IREnum* st)
{
	llvm::Type* llvm_max_type_by_size = nullptr;
	int max_size = 0;

	llvm::Type* llvm_current_type = nullptr;
	int current_type_size = 0;
	for( auto& member : *st->members )
	{
		auto enum_member = member.second;

		if( enum_member->contained_type == ir::IREnumMember::Type::Struct )
		{
			auto cg = codegen_struct(enum_member->struct_member);
			if( !cg.ok() )
				return cg;

			auto typer = get_type(
				*this, sema::TypeInstance::OfType(enum_member->struct_member->struct_type));
			if( !typer.ok() )
				return typer;
			llvm_current_type = typer.unwrap();
			auto llvm_size = Module->getDataLayout().getTypeAllocSize(llvm_current_type);
			current_type_size = llvm_size.getKnownMinSize();
		}
		else
		{
			// Empty fields are 0.
		}

		if( current_type_size > max_size )
		{
			llvm_max_type_by_size = llvm_current_type;
			max_size = current_type_size;
		}
	}
	Vec<llvm::Type*> members = {llvm::Type::getInt32Ty(*Context)};
	if( llvm_max_type_by_size != nullptr )
	{
		members.push_back(llvm_max_type_by_size);
	}

	auto enum_type = st->enum_type;
	auto name = enum_type->get_name();
	llvm::StructType* llvm_struct_type = llvm::StructType::create(*Context, members, name);

	this->types.emplace(enum_type, llvm_struct_type);

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