#include "Codegen.h"

#include "Expr.h"
#include "emit.h"
#include <llvm/IR/IRBuilder.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

Codegen::Codegen(SymBuiltins& builtins)
	: context(std::make_unique<llvm::LLVMContext>())
	, mod(std::make_unique<llvm::Module>("this_module", *context))
	, builder(std::make_unique<llvm::IRBuilder<>>(*context))
{
	tys.emplace(builtins.i8_ty, llvm::Type::getInt8Ty(*context));
	tys.emplace(builtins.i16_ty, llvm::Type::getInt16Ty(*context));
	tys.emplace(builtins.i32_ty, llvm::Type::getInt32Ty(*context));
	tys.emplace(builtins.i64_ty, llvm::Type::getInt64Ty(*context));
	tys.emplace(builtins.u8_ty, llvm::Type::getInt8Ty(*context));
	tys.emplace(builtins.u16_ty, llvm::Type::getInt16Ty(*context));
	tys.emplace(builtins.u32_ty, llvm::Type::getInt32Ty(*context));
	tys.emplace(builtins.u64_ty, llvm::Type::getInt8Ty(*context));

	tys.emplace(builtins.void_ty, llvm::Type::getVoidTy(*context));
	tys.emplace(builtins.bool_ty, llvm::Type::getInt1Ty(*context));
}

void
Codegen::print()
{
	std::string str;
	llvm::raw_string_ostream out(str);

	mod->print(out, nullptr);

	std::cout << str;
}

int
Codegen::emit()
{
	return ::emit(mod.get());
}

Codegen
Codegen::codegen(SymBuiltins& builtins, HirNode* module)
{
	Codegen gen(builtins);

	gen.codegen_module(module);

	return gen;
}

Expr
Codegen::codegen_module(HirNode* hir_mod)
{
	HirModule& mod = hir_cast<HirModule>(hir_mod);

	for( auto stmt : mod.statements )
		codegen_item(stmt);

	return Expr::Empty();
}

Expr
Codegen::codegen_item(HirNode* hir_item)
{
	switch( hir_item->kind )
	{
	case HirNodeKind::Func:
		return codegen_func(hir_item);
	case HirNodeKind::FuncProto:
		codegen_func_proto(hir_item);
		return Expr::Empty();
	case HirNodeKind::Struct:
		codegen_struct(hir_item);
		return Expr::Empty();
	default:
		return Expr::Empty();
	}
}

Expr
Codegen::codegen_struct(HirNode* hir_struct)
{
	HirStruct& struct_nod = hir_cast<HirStruct>(hir_struct);
	SymType& sym_type = sym_cast<SymType>(struct_nod.sym);
	TyStruct const& ty_struct = ty_cast<TyStruct>(sym_type.ty);

	std::vector<llvm::Type*> members;
	for( auto& [name, member] : ty_struct.members )
		members.push_back(get_type(member.qty));

	// TODO: Need unique name for syms
	std::string name_str = ty_struct.name;
	llvm::StructType* llvm_struct_type = llvm::StructType::create(*context, members, name_str);

	tys.emplace(sym_type.ty, llvm_struct_type);

	return Expr::Empty();
}

static TyFunc const&
func_ty(Sym* sym)
{
	SymFunc& func = sym_cast<SymFunc>(sym);
	TyFunc const& func_ty = ty_cast<TyFunc>(func.ty);

	return func_ty;
}

static std::string
func_name(Sym* sym)
{
	// Eventually mangle names.
	return func_ty(sym).name;
}

static llvm::Function::LinkageTypes
linkage(HirFuncProto::Linkage link)
{
	switch( link )
	{
	case HirFuncProto::Linkage::Extern:
		return llvm::Function::ExternalLinkage;
	default:
		return llvm::Function::PrivateLinkage;
	}
}

Expr
Codegen::codegen_func(HirNode* hir_func)
{
	//
	HirFunc& func_nod = hir_cast<HirFunc>(hir_func);

	Function* func = codegen_func_proto(func_nod.proto);
	current_func = func;

	llvm::Function* llvm_fn = func->llvm_func;
	llvm::BasicBlock* llvm_entry_bb = llvm::BasicBlock::Create(*context, "", llvm_fn);
	builder->SetInsertPoint(llvm_entry_bb);

	for( int i = 0; i < func->ir_arg_count(); i++ )
	{
		Arg& arg = func->ir_arg(i);
		assert(arg.sym);

		int llvm_arg_index = func->llvm_arg_index(i);
		llvm::Value* val = nullptr;

		if( arg.is_byval() )
			val = llvm_fn->getArg(llvm_arg_index);
		else
		{
			val = builder->CreateAlloca(arg.type);
			builder->CreateStore(llvm_fn->getArg(llvm_arg_index), val);
		}

		vars.emplace(arg.sym, Address(val, arg.type));
	}

	codegen_block(func_nod.body);

	vars.clear();
	current_func = nullptr;

	return Expr::Empty();
}

static std::vector<llvm::Type*>
to_llvm_arg_tys(std::vector<Arg> const& args)
{
	std::vector<llvm::Type*> arg_tys;
	for( Arg const& arg : args )
	{
		if( arg.is_byval() || arg.is_sret() )
			arg_tys.push_back(arg.type->getPointerTo());
		else
			arg_tys.push_back(arg.type);
	}
	return arg_tys;
}

static Sym*
param_sym(HirNode* hir_param)
{
	HirId& id = hir_cast<HirId>(hir_param);

	return id.sym;
}

Function*
Codegen::codegen_func_proto(HirNode* hir_proto)
{
	HirFuncProto& proto = hir_cast<HirFuncProto>(hir_proto);

	TyFunc const& ty_func = func_ty(proto.sym);

	llvm::Type* ret_ty = get_type(ty_func.rt_qty);
	std::vector<Arg> args;

	if( ty_func.rt_qty.is_aggregate_type() )
	{
		args.push_back(Arg(nullptr, ret_ty, true, false));
		ret_ty = llvm::Type::getVoidTy(*context);
	}

	for( int i = 0; i < ty_func.args_qtys.size(); i++ )
	{
		Sym* sym = param_sym(proto.parameters.at(i));
		QualifiedTy qty = ty_func.args_qtys.at(i);
		llvm::Type* ty = get_type(qty);
		args.push_back(Arg(sym, ty, false, qty.is_aggregate_type()));
	}

	llvm::FunctionType* llvm_fn_ty = llvm::FunctionType::get(
		ret_ty, to_llvm_arg_tys(args), proto.var_arg == HirFuncProto::VarArg::VarArg);

	llvm::Function* llvm_fn =
		llvm::Function::Create(llvm_fn_ty, linkage(proto.linkage), func_name(proto.sym), mod.get());

	for( int i = 0; i < args.size(); i++ )
	{
		Arg& arg = args.at(i);
		if( arg.is_sret() )
			llvm_fn->getArg(i)->addAttrs(llvm::AttrBuilder().addStructRetAttr(arg.type));
		else if( arg.is_byval() )
			llvm_fn->getArg(i)->addAttrs(llvm::AttrBuilder().addByValAttr(arg.type));
	}

	auto emplaced = funcs.emplace(proto.sym, Function::FromArgs(llvm_fn, args));
	return &emplaced.first->second;
}

Expr
Codegen::codegen_construct(HirNode* hir_construct)
{
	HirConstruct& construct = hir_cast<HirConstruct>(hir_construct);

	Expr lhs = codegen_expr(construct.self);

	Expr called = codegen_expr(construct.call, lhs);

	if( llvm::Value* value = codegen_eval(called); !value->getType()->isVoidTy() )
		builder->CreateStore(value, lhs.address().llvm_pointer());

	return Expr::Empty();
}

Expr
Codegen::codegen_func_call(HirNode* hir_call, Expr sret)
{
	HirFuncCall& call = hir_cast<HirFuncCall>(hir_call);
	assert(call.kind != HirFuncCall::CallKind::Invalid);

	switch( call.kind )
	{
	case HirFuncCall::CallKind::Static:
		return codegen_func_call_static(hir_call, sret);
	default:
		return Expr::Empty();
	}
}

Expr
Codegen::codegen_func_call_static(HirNode* hir_call, Expr sret)
{
	HirFuncCall& call_nod = hir_cast<HirFuncCall>(hir_call);
	assert(call_nod.kind == HirFuncCall::CallKind::Static);

	Function& callee = funcs.at(call_nod.callee);

	std::vector<llvm::Value*> llvm_arg_values;
	if( !callee.sret.is_void() )
	{
		Address sret_arg = callee.sret.address();

		// Allocate the sret arg if one is not provided.
		llvm::Value* val;
		if( sret.is_address() )
			val = sret.address().llvm_pointer();
		else
			val = builder->CreateAlloca(sret_arg.llvm_allocated_type());

		llvm_arg_values.push_back(val);
	}

	for( int i = 0; i < callee.ir_arg_count(); i++ )
	{
		HirNode* hir_expr = call_nod.args.at(i);
		Arg& arg = callee.ir_arg(i);
		Expr expr = codegen_expr(hir_expr);

		if( arg.is_byval() )
		{
			llvm::Value* val = builder->CreateAlloca(arg.type);

			codegen_memcpy(expr, val);

			llvm_arg_values.push_back(val);
		}
		else
		{
			llvm_arg_values.push_back(codegen_eval(expr));
		}
	}

	// Var args.
	for( int i = callee.ir_arg_count(); i < call_nod.args.size(); i++ )
	{
		HirNode* hir_expr = call_nod.args.at(i);
		Expr expr = codegen_expr(hir_expr);
		llvm_arg_values.push_back(codegen_eval(expr));
	}

	llvm::Value* call = builder->CreateCall(callee.llvm_func, llvm_arg_values);

	return RValue(call);
}

static llvm::Value*
pointer_of(Expr expr)
{
	llvm::Value* llvm_lhs =
		expr.is_address() ? expr.address().llvm_pointer() : expr.rvalue().llvm_pointer();
	return llvm_lhs;
}

Expr
Codegen::codegen_binop(HirNode* hir_call)
{
	HirBinOp& call = hir_cast<HirBinOp>(hir_call);
	Expr lhs_expr = codegen_expr(call.lhs);
	Expr rhs_expr = codegen_expr(call.rhs);

	llvm::Value* llvm_rhs = codegen_eval(rhs_expr);

	if( llvm::Value* llvm_lhs = pointer_of(lhs_expr); call.op == BinOp::Assign )
		return Expr(RValue(builder->CreateStore(llvm_rhs, llvm_lhs)));

	llvm::Value* llvm_lhs = codegen_eval(lhs_expr);

	switch( call.op )
	{
	case BinOp::Mul:
		return Expr(RValue(builder->CreateNSWMul(llvm_lhs, llvm_rhs)));
	case BinOp::Div:
		return Expr(RValue(builder->CreateSDiv(llvm_lhs, llvm_rhs)));
	case BinOp::Add:
		return Expr(RValue(builder->CreateNSWAdd(llvm_lhs, llvm_rhs)));
	case BinOp::Sub:
		return Expr(RValue(builder->CreateNSWSub(llvm_lhs, llvm_rhs)));
	case BinOp::Gt:
		return Expr(RValue(builder->CreateICmpSGT(llvm_lhs, llvm_rhs)));
	case BinOp::Gte:
		return Expr(RValue(builder->CreateICmpSGE(llvm_lhs, llvm_rhs)));
	case BinOp::Lt:
		return Expr(RValue(builder->CreateICmpSLT(llvm_lhs, llvm_rhs)));
	case BinOp::Lte:
		return Expr(RValue(builder->CreateICmpSLE(llvm_lhs, llvm_rhs)));
	case BinOp::Eq:
		return Expr(RValue(builder->CreateICmpEQ(llvm_lhs, llvm_rhs)));
	case BinOp::Neq:
		return Expr(RValue(builder->CreateICmpNE(llvm_lhs, llvm_rhs)));
	default:
		return Expr::Empty();
	}
}

Expr
Codegen::codegen_builtin(HirNode* hir_call)
{
	HirBuiltin& call = hir_cast<HirBuiltin>(hir_call);

	switch( call.builtin )
	{
	case HirBuiltin::BuiltinKind::IntCast:
		return codegen_intcast(hir_call);
	case HirBuiltin::BuiltinKind::Deref:
		return codegen_deref(hir_call);
	case HirBuiltin::BuiltinKind::AddressOf:
		return codegen_addressof(hir_call);
	default:
		return Expr::Empty();
	}
}

Expr
Codegen::codegen_intcast(HirNode* hir_call)
{
	HirBuiltin& call = hir_cast<HirBuiltin>(hir_call);
	assert(call.builtin == HirBuiltin::BuiltinKind::IntCast);

	HirId& id = hir_cast<HirId>(call.args.at(0));
	Ty const* ty = sym_cast<SymType>(id.sym).ty;
	llvm::Type* dest = get_type(ty);

	Expr value_expr = codegen_expr(call.args.at(1));
	llvm::Value* value = codegen_eval(value_expr);
	assert(value);

	TyInt const& ty_int = ty_cast<TyInt>(ty);

	switch( ty_int.sign )
	{
	case TyInt::Sign::Signed:
	{
		if( value->getType()->getIntegerBitWidth() < dest->getIntegerBitWidth() )
			// S is for Signed-Extend
			return RValue(builder->CreateSExt(value, dest));
		else
			return RValue(builder->CreateTrunc(value, dest));
	}
	case TyInt::Sign::Unsigned:
	{
		if( value->getType()->getIntegerBitWidth() < dest->getIntegerBitWidth() )
			// Z is for Zero-Extend
			return RValue(builder->CreateZExt(value, dest));
		else
			return RValue(builder->CreateTrunc(value, dest));
	}
	case TyInt::Sign::Any:
		return Expr::Empty();
	}
}

Expr
Codegen::codegen_deref(HirNode* hir_call)
{
	HirBuiltin& call = hir_cast<HirBuiltin>(hir_call);
	assert(call.builtin == HirBuiltin::BuiltinKind::Deref);

	Expr dest = codegen_expr(call.args.at(0));

	llvm::Value* ptr = codegen_eval(dest);
	llvm::Type* ty = get_type(hir_call->qty);

	return Address(ptr, ty);
}

Expr
Codegen::codegen_addressof(HirNode* hir_call)
{
	HirBuiltin& call = hir_cast<HirBuiltin>(hir_call);
	assert(call.builtin == HirBuiltin::BuiltinKind::AddressOf);

	Expr dest = codegen_expr(call.args.at(0));

	return RValue(dest.address().llvm_pointer(), get_type(hir_call->qty));
}

static QualifiedTy
ty_ty(Sym* sym)
{
	SymType& func = sym_cast<SymType>(sym);

	return func.ty;
}

Expr
Codegen::codegen_member_access(HirNode* hir_ma)
{
	HirMember& member = hir_cast<HirMember>(hir_ma);

	Address self = codegen_expr(member.self).address();

	SymMember& mem = sym_cast<SymMember>(member.member);

	llvm::Value* val =
		builder->CreateStructGEP(self.llvm_allocated_type(), self.llvm_pointer(), mem.member.ind);

	llvm::Type* mem_ty = get_type(mem.member.qty);
	return Expr(Address(val, mem_ty));
}

Expr
Codegen::codegen_block(HirNode* hir_block)
{
	HirBlock& block = hir_cast<HirBlock>(hir_block);

	Expr expr = Expr::Empty();
	for( auto& stmt : block.statements )
		expr = codegen_expr(stmt);

	return expr;
}

static QualifiedTy
var_ty(Sym* sym)
{
	SymVar& func = sym_cast<SymVar>(sym);

	return func.qty;
}

Expr
Codegen::codegen_let(HirNode* hir_let)
{
	HirLet& let = hir_cast<HirLet>(hir_let);

	llvm::Type* type = get_type(var_ty(let.sym));

	llvm::Value* val = builder->CreateAlloca(type);

	vars.emplace(let.sym, Address(val, type));

	return Expr::Empty();
}

Expr
Codegen::codegen_if(HirNode* hir_if)
{
	HirIf& if_nod = hir_cast<HirIf>(hir_if);

	if( if_nod.cond_else )
		return codegen_if_phi(hir_if);
	else
		return codegen_if_chain(hir_if);
}

Expr
Codegen::codegen_if_chain(HirNode* hir_if)
{
	HirIf& if_nod = hir_cast<HirIf>(hir_if);
	assert(!if_nod.cond_else);

	llvm::Function* llvm_fn = current_func->llvm_func;
	llvm::BasicBlock* llvm_cond_bb = builder->GetInsertBlock();
	llvm::BasicBlock* llvm_then_bb = nullptr;
	llvm::BasicBlock* llvm_else_bb = nullptr;
	llvm::BasicBlock* llvm_merge_bb = llvm::BasicBlock::Create(*context, "");

	for( int i = 0; i < if_nod.elsifs.size(); i++ )
	{
		HirIf::CondThen& cond_then = if_nod.elsifs.at(i);

		Expr cond = codegen_expr(cond_then.cond);
		llvm::Value* cond_val = codegen_eval(cond);

		llvm_then_bb = llvm::BasicBlock::Create(*context, "", llvm_fn);
		llvm_else_bb = llvm::BasicBlock::Create(*context, "", llvm_fn);
		builder->CreateCondBr(cond_val, llvm_then_bb, llvm_else_bb);

		builder->SetInsertPoint(llvm_then_bb);

		codegen_expr(cond_then.then);
		builder->CreateBr(llvm_merge_bb);

		builder->SetInsertPoint(llvm_else_bb);
	}

	if( if_nod.else_node )
	{
		codegen_expr(if_nod.else_node);
		builder->CreateBr(llvm_merge_bb);
	}

	llvm_merge_bb->insertInto(current_func->llvm_func);
	builder->SetInsertPoint(llvm_merge_bb);

	return Expr::Empty();
}

/**
 * Used for short circuiting boolean expressions
 * Only supports 1 chain.
 *
 * e.g.
 * bool val = a || b
 *
 * yields something like
 *
 * %1 = a
 * if (!val)
 *  %2 = b
 * val = phi %1 %2
 *
 *
 * @return Expr
 */
Expr
Codegen::codegen_if_phi(HirNode* hir_if)
{
	HirIf& if_nod = hir_cast<HirIf>(hir_if);
	assert(!if_nod.else_node && if_nod.cond_else && if_nod.elsifs.size() == 1);

	llvm::Function* llvm_fn = current_func->llvm_func;
	llvm::BasicBlock* llvm_cond_bb = builder->GetInsertBlock();
	llvm::BasicBlock* llvm_else_bb = llvm::BasicBlock::Create(*context, "", llvm_fn);
	llvm::BasicBlock* llvm_merge_bb = llvm::BasicBlock::Create(*context, "", llvm_fn);

	HirIf::CondThen& cond_then = if_nod.elsifs.at(0);
	Expr cond = codegen_expr(cond_then.cond);
	llvm::Value* cond_val = codegen_eval(cond);

	builder->CreateCondBr(cond_val, llvm_merge_bb, llvm_else_bb);

	builder->SetInsertPoint(llvm_else_bb);
	Expr then = codegen_expr(cond_then.then);
	llvm::Value* then_val = codegen_eval(then);
	builder->CreateBr(llvm_merge_bb);

	builder->SetInsertPoint(llvm_merge_bb);
	llvm::PHINode* phi = builder->CreatePHI(llvm::Type::getInt1Ty(*context), 2);
	phi->addIncoming(cond_val, llvm_cond_bb);
	phi->addIncoming(then_val, llvm_else_bb);

	return RValue(phi);
}

Expr
Codegen::codegen_var(HirNode* hir_id)
{
	HirId& var = hir_cast<HirId>(hir_id);

	auto iter_vars = vars.find(var.sym);
	assert(iter_vars != vars.end());
	return Expr(iter_vars->second);
}

// TODO: This should probably just be HirMember with nullable self
// Expr
// Codegen::codegen_var_member(HirNode* hir_id)
// {
// 	assert(current_this.is_address());
// 	HirId& var = hir_cast<HirId>(hir_id);

// 	SymMember& mem = sym_cast<SymMember>(var.sym);

// 	llvm::Value* val = builder->CreateStructGEP(
// 		current_this.address().llvm_allocated_type(),
// 		current_this.address().llvm_pointer(),
// 		mem.member.ind);

// 	llvm::Type* mem_ty = get_type(mem.member.qty);
// 	return Expr(Address(val, mem_ty));
// }
Expr
Codegen::codegen_expr(HirNode* hir_expr)
{
	return codegen_expr(hir_expr, Expr::Empty());
}

Expr
Codegen::codegen_expr(HirNode* hir_expr, Expr lhs)
{
	switch( hir_expr->kind )
	{
	case HirNodeKind::NumberLiteral:
		return codegen_number_literal(hir_expr);
	case HirNodeKind::StringLiteral:
		return codegen_string_literal(hir_expr);
	case HirNodeKind::Construct:
		return codegen_construct(hir_expr);
	case HirNodeKind::FuncCall:
		return codegen_func_call(hir_expr, lhs);
	case HirNodeKind::BinOp:
		return codegen_binop(hir_expr);
	case HirNodeKind::Builtin:
		return codegen_builtin(hir_expr);
	case HirNodeKind::Id:
		return codegen_var(hir_expr);
	case HirNodeKind::Return:
		return codegen_return(hir_expr);
	case HirNodeKind::If:
		return codegen_if(hir_expr);
	case HirNodeKind::Let:
		return codegen_let(hir_expr);
	case HirNodeKind::Block:
		return codegen_block(hir_expr);
	case HirNodeKind::Member:
		return codegen_member_access(hir_expr);
	default:
		return Expr::Empty();
	}
}

Expr
Codegen::codegen_return(HirNode* hir_return)
{
	HirReturn& ret = hir_cast<HirReturn>(hir_return);

	Expr expr = codegen_expr(ret.expr);

	if( !current_func->sret.is_void() )
	{
		llvm::Value* dest = current_func->sret.address().llvm_pointer();
		codegen_memcpy(expr, dest);

		builder->CreateRetVoid();
		return Expr::Empty();
	}

	if( expr.is_void() )
	{
		builder->CreateRetVoid();
	}
	else
	{
		auto llvm_value = codegen_eval(expr);
		assert(!llvm_value->getType()->isAggregateType());
		builder->CreateRet(llvm_value);
	}

	return Expr::Empty();
}

Expr
Codegen::codegen_number_literal(HirNode* hir_nl)
{
	HirNumberLiteral nl = hir_cast<HirNumberLiteral>(hir_nl);
	llvm::Value* llvm_const_int = llvm::ConstantInt::get(*context, llvm::APInt(32, nl.value, true));

	return Expr(RValue(llvm_const_int, llvm_const_int->getType()));
}

static char
escape_char(char c)
{
	// \a	07	Alert (Beep, Bell) (added in C89)[1]
	// \b	08	Backspace
	// \e	1B	Escape character
	// \f	0C	Formfeed Page Break
	// \n	0A	Newline (Line Feed); see notes below
	// \r	0D	Carriage Return
	// \t	09	Horizontal Tab
	// \v	0B	Vertical Tab
	// \\	5C	Backslash
	// \'	27	Apostrophe or single quotation mark
	// \"	22	Double quotation mark
	// \?	3F	Question mark (used to avoid trigraphs)

	switch( c )
	{
	case 'a':
		return 0x07;
	case 'b':
		return 0x08;
	case 'e':
		return 0x1B;
	case 'f':
		return 0x0C;
	case 'n':
		return 0x0A;
	case 'r':
		return 0x0D;
	case 't':
		return 0x09;
	case 'v':
		return 0x0B;
	case '\\':
		return '\\';
	case '\'':
		return '\'';
	case '"':
		return '"';
	case '?':
		return '?';
	default:
		return c;
	}
}

static std::string
escape_string(std::string s)
{
	std::string res;
	res.reserve(s.size());
	bool escape = false;
	for( auto c : s )
	{
		if( !escape && c == '\\' )
		{
			escape = true;
			continue;
		}

		res.push_back(escape ? escape_char(c) : c);
		escape = false;
	}

	return res;
}

Expr
Codegen::codegen_string_literal(HirNode* hir_str)

{
	HirStringLiteral& str = hir_cast<HirStringLiteral>(hir_str);

	auto llvm_literal =
		llvm::ConstantDataArray::getString(*context, escape_string(str.value).c_str(), true);

	llvm::GlobalVariable* llvm_global = new llvm::GlobalVariable(
		*mod, llvm_literal->getType(), true, llvm::GlobalValue::InternalLinkage, llvm_literal);
	llvm::Constant* zero = llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(*context));
	llvm::Constant* indices[] = {zero, zero};
	llvm::Constant* llvm_str =
		llvm::ConstantExpr::getGetElementPtr(llvm_literal->getType(), llvm_global, indices);

	return RValue(llvm_str);
}

llvm::Value*
Codegen::codegen_memcpy(Expr expr, llvm::Value* dest)
{
	assert(expr.is_address());
	Address src_address = expr.address();

	auto llvm_size = mod->getDataLayout().getTypeAllocSize(src_address.llvm_allocated_type());
	auto llvm_align = mod->getDataLayout().getPrefTypeAlign(src_address.llvm_allocated_type());

	builder->CreateMemCpy(dest, llvm_align, src_address.llvm_pointer(), llvm_align, llvm_size);

	return dest;
}

llvm::Value*
Codegen::codegen_eval(Expr expr)
{
	if( expr.is_address() )
	{
		Address address = expr.address();
		return builder->CreateLoad(address.llvm_allocated_type(), address.llvm_pointer());
	}
	else if( expr.is_rvalue() )
	{
		RValue rvalue = expr.rvalue();
		return rvalue.llvm_pointer();
	}
	else
	{
		assert("Invalid eval.");
		return nullptr;
	}
}

llvm::Type*
Codegen::get_type(QualifiedTy qty)
{
	llvm::Type* base = get_type(qty.ty);

	for( int i = 0; i < qty.indirection; i++ )
		base = base->getPointerTo();

	return base;
}

llvm::Type*
Codegen::get_type(Ty const* ty)
{
	return tys[ty];
}