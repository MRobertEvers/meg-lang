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
		codegen_sync_proto(hir_item);
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

	std::cout << "Struct Type: " << name_str << " " << std::hex << llvm_struct_type << std::endl;

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

	HirFuncProto& proto = hir_cast<HirFuncProto>(func_nod.proto);
	switch( proto.kind )
	{
	case HirFuncProto::Routine::Coroutine:
		return codegen_async_func(hir_func);
	case HirFuncProto::Routine::Subroutine:
		return codegen_sync_func(hir_func);
	}
}

void
Codegen::codegen_func_entry(Function* func)
{
	// This only generates the entry code for ir args.
	// codegened implicit args should be handled separately.
	llvm::Function* llvm_fn = func->llvm_func;
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

		vars.add(arg.sym, Address(val, arg.type));
	}
}

Expr
Codegen::codegen_sync_func(HirNode* hir_func)
{
	HirFunc& func_nod = hir_cast<HirFunc>(hir_func);

	Function* func = codegen_sync_proto(func_nod.proto);
	current_func = func;

	llvm::Function* llvm_fn = func->llvm_func;
	llvm::BasicBlock* llvm_entry_bb = llvm::BasicBlock::Create(*context, "", llvm_fn);
	builder->SetInsertPoint(llvm_entry_bb);

	codegen_func_entry(func);

	codegen_block(func_nod.body);

	vars.clear();
	current_func = nullptr;

	return Expr::Empty();
}

Expr
Codegen::codegen_async_func(HirNode* hir_func)
{
	HirFunc& func = hir_cast<HirFunc>(hir_func);
	HirNode* hir_proto = func.proto;

	HirFuncProto& proto = hir_cast<HirFuncProto>(hir_proto);

	// Prepare the struct
	codegen_async_frame_t frame = codegen_async_frame(hir_proto);

	codegen_async_constructor(hir_proto, frame);

	// Return [llvm_fn_step, llvm_optional_send_ty];
	codegen_async_step_t async_step = codegen_async_step(hir_func, frame);

	codegen_async_begin(hir_proto, frame.frame_ty, async_step);
	codegen_async_send(hir_proto, frame.frame_ty, async_step);

	codegen_async_close(hir_proto, frame.frame_ty);

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
Codegen::codegen_sync_proto(HirNode* hir_proto)
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

Codegen::codegen_async_frame_t
Codegen::codegen_async_frame(HirNode* hir_proto)
{
	// // Codegen Frame
	//
	// struct Frame<TRet> {
	//     i32 step;
	//	   Ret value;
	//	   ...Args value;
	//     ...Locals value;
	// }
	//
	std::map<Sym*, int> lut;
	HirFuncProto& proto = hir_cast<HirFuncProto>(hir_proto);
	SymFunc& proto_sym = sym_cast<SymFunc>(proto.sym);
	SymType& proto_impl_sym = sym_cast<SymType>(proto.impl_sym);
	TyFunc const& ty_func = ty_cast<TyFunc>(proto_sym.ty);

	std::vector<llvm::Type*> members;

	llvm::Type* llvm_step_idx_type = llvm::Type::getInt32Ty(*context);
	members.push_back(llvm_step_idx_type);

	SymType& close_ty_sym = sym_cast<SymType>(sym_unalias(proto_impl_sym.scope.find("Ret")));

	llvm::Type* llvm_ret_ty = get_type(close_ty_sym.ty);
	members.push_back(llvm_ret_ty);

	for( auto& local : proto.parameters )
	{
		HirId& id_nod = hir_cast<HirId>(local);
		llvm::Type* llvm_local_type = get_type(local->qty);
		lut.emplace(id_nod.sym, members.size());
		members.push_back(llvm_local_type);
	}

	for( auto& local : proto.locals )
	{
		HirLet& let_nod = hir_cast<HirLet>(local);
		llvm::Type* llvm_local_type = get_type(sym_cast<SymVar>(let_nod.sym).qty);
		lut.emplace(let_nod.sym, members.size());
		members.push_back(llvm_local_type);
	}

	// async_fn.return_val_idx = members.size() - 1;

	std::string result_struct_name = "TEMP";
	// result_struct_name += async_fn.sema_return_type.type->get_name();
	// result_struct_name += ">";

	llvm::StructType* llvm_struct_type =
		llvm::StructType::create(*context, members, result_struct_name.c_str());

	std::cout << "Frame Type: " << std::hex << llvm_struct_type << std::endl;
	tys.emplace(proto_impl_sym.ty, llvm_struct_type);

	return codegen_async_frame_t{
		.frame_ty = llvm_struct_type,
		.step_frame_idx = 0,
		.ret_frame_idx = 1,
		.sym_frame_idx_lut = lut};
}

Expr
Codegen::codegen_async_constructor(HirNode* hir_proto, codegen_async_frame_t frame)
{
	llvm::Type* llvm_frame_ty = frame.frame_ty;
	HirFuncProto& proto = hir_cast<HirFuncProto>(hir_proto);
	// The constructor uses the symbol of the function itself
	Sym* proto_sym = proto.sym;

	TyFunc const& ty_func = func_ty(proto_sym);

	std::vector<Arg> args;

	args.push_back(Arg(nullptr, llvm_frame_ty, true, false));
	llvm::Type* llvm_ret_ty = llvm::Type::getVoidTy(*context);

	for( int i = 0; i < ty_func.args_qtys.size(); i++ )
	{
		Sym* sym = param_sym(proto.parameters.at(i));
		QualifiedTy qty = ty_func.args_qtys.at(i);
		llvm::Type* ty = get_type(qty);
		args.push_back(Arg(sym, ty, false, qty.is_aggregate_type()));
	}

	llvm::FunctionType* llvm_fn_ty = llvm::FunctionType::get(
		llvm_ret_ty, to_llvm_arg_tys(args), proto.var_arg == HirFuncProto::VarArg::VarArg);

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
	current_func = &emplaced.first->second;

	llvm::BasicBlock* llvm_entry_bb = llvm::BasicBlock::Create(*context, "entry", llvm_fn);
	builder->SetInsertPoint(llvm_entry_bb);

	codegen_func_entry(current_func);

	for( auto [sym, alloca_addr] : vars )
	{
		int frame_idx = frame.sym_frame_idx_lut.at(sym);
		llvm::Value* llvm_gep =
			builder->CreateStructGEP(llvm_frame_ty, llvm_fn->getArg(0), frame_idx);

		codegen_memcpy(*alloca_addr, llvm_gep);
	}

	builder->CreateRetVoid();

	vars.clear();
	current_func = nullptr;

	return Expr::Empty();
}

// CGExpr
// cg::cg_copy(CG& codegen, Address& src, Address& dest)
// {
// 	auto src_address = src.fixup();

// 	auto llvm_size =
// 		codegen.Module->getDataLayout().getTypeAllocSize(src_address.llvm_allocated_type());
// 	auto llvm_align =
// 		codegen.Module->getDataLayout().getPrefTypeAlign(src_address.llvm_allocated_type());

// 	codegen.Builder->CreateMemCpy(
// 		dest.llvm_pointer(), llvm_align, src_address.llvm_pointer(), llvm_align, llvm_size);

// 	auto maybe_fixup_info = src.fixup_info();
// 	if( maybe_fixup_info.has_value() )
// 		return cg_fixdown(codegen, dest, maybe_fixup_info.value()).unwrap();
// 	else
// 		return CGExpr::MakeAddress(dest);
// }

Expr
Codegen::codegen_async_step_rehydration(
	HirNode* hir_func, llvm::BasicBlock* llvm_entry_bb, codegen_async_frame_t frame)
{
	// Copy values from the frame onto the stack
	assert(current_func);

	llvm::Type* llvm_frame_type = frame.frame_ty;

	llvm::Function* llvm_fn = current_func->llvm_func;
	// llvm::BasicBlock* llvm_rehydration_bb =
	// 	llvm::BasicBlock::Create(*context, "Rehydration", llvm_fn, llvm_entry_bb);
	// builder->SetInsertPoint(llvm_rehydration_bb);

	// TODO: Perhaps we don't need to do any rehydration and can just access variables in the
	// frame...

	// llvm::Argument* llvm_frame_arg = llvm_fn->getArg(1);

	// // Use the frame lookup to get the idx of the symbol in the frame,
	// // then copy the value to the stack.
	// for( auto [sym, alloca_addr] : vars )
	// {
	// 	int frame_idx = frame.sym_frame_idx_lut.at(sym);
	// 	llvm::Value* llvm_gep =
	// 		builder->CreateStructGEP(llvm_frame_type, llvm_frame_arg, frame_idx);

	// 	codegen_memcpy(
	// 		Expr(Address(llvm_gep, alloca_addr->llvm_allocated_type())),
	// 		alloca_addr->llvm_pointer());
	// }

	return Expr::Empty();
}

llvm::SwitchInst*
Codegen::codegen_async_step_jump_table(
	HirNode*,
	llvm::BasicBlock* llvm_entry_bb,
	llvm::BasicBlock* llvm_body_block,
	codegen_async_frame_t frame)
{
	// Must come after rehydration
	assert(current_func);

	llvm::Type* llvm_frame_type = frame.frame_ty;

	llvm::Function* llvm_fn = current_func->llvm_func;
	llvm::BasicBlock* llvm_jt_bb =
		llvm::BasicBlock::Create(*context, "JT", llvm_fn, llvm_body_block);

	builder->SetInsertPoint(llvm_entry_bb);
	builder->CreateBr(llvm_jt_bb);
	builder->SetInsertPoint(llvm_jt_bb);

	llvm::Argument* llvm_frame_arg = llvm_fn->getArg(1);

	// Deref first element of from frame.
	llvm::Value* llvm_step_value_ptr =
		builder->CreateStructGEP(llvm_frame_type, llvm_frame_arg, frame.step_frame_idx);

	llvm::Value* llvm_step_value =
		builder->CreateLoad(llvm::Type::getInt32Ty(*context), llvm_step_value_ptr);

	// Basic Block for when we send after the coro is done.
	llvm::BasicBlock* llvm_bad_send_bb = llvm::BasicBlock::Create(*context, "BadSend", llvm_fn);
	// TODO: Crash here.
	builder->SetInsertPoint(llvm_bad_send_bb);
	builder->CreateRetVoid();
	builder->SetInsertPoint(llvm_jt_bb);

	llvm::SwitchInst* llvm_switch = builder->CreateSwitch(llvm_step_value, llvm_bad_send_bb, 4);

	llvm::ConstantInt* llvm_jump_idx = llvm::ConstantInt::get(*context, llvm::APInt(32, 0, true));

	// On first call, just jump to the body.
	llvm_switch->addCase(llvm_jump_idx, llvm_body_block);

	int jump_idx = 1;
	for( auto yield_point : current_func->yield_points )
	{
		llvm_jump_idx = llvm::ConstantInt::get(*context, llvm::APInt(32, jump_idx, true));

		llvm_switch->addCase(llvm_jump_idx, yield_point.resume_bb);
		jump_idx += 1;
	}

	return llvm_switch;
}

Expr
Codegen::codegen_async_step_suspends_backpatch(
	HirNode*, codegen_async_step_t step, codegen_async_frame_t frame)
{
	llvm::Function* llvm_fn = current_func->llvm_func;

	llvm::Type* llvm_frame_type = frame.frame_ty;
	llvm::Type* llvm_send_return_type = step.iter_ret_ty;

	llvm::ConstantInt* llvm_zero = llvm::ConstantInt::get(*context, llvm::APInt(1, 0, true));

	int yield_idx = 0;
	for( auto yield_point : current_func->yield_points )
	{
		llvm::BasicBlock* llvm_suspend_block = yield_point.suspend_bb;
		builder->SetInsertPoint(llvm_suspend_block);

		llvm::Argument* llvm_frame_arg = llvm_fn->getArg(1);
		llvm::Value* llvm_step_value_ptr =
			builder->CreateStructGEP(llvm_frame_type, llvm_frame_arg, frame.step_frame_idx);

		llvm::Argument* llvm_send_return_arg = llvm_fn->getArg(0);
		llvm::Value* llvm_send_return_done_ptr =
			builder->CreateStructGEP(llvm_send_return_type, llvm_send_return_arg, 0);
		llvm::Value* llvm_send_return_value_ptr =
			builder->CreateStructGEP(llvm_send_return_type, llvm_send_return_arg, 1);

		// Set done to false in the return value.
		builder->CreateStore(llvm_zero, llvm_send_return_done_ptr);
		// Set the value in the return value.
		if( !yield_point.yield_expr.is_void() )
			codegen_memcpy(yield_point.yield_expr.address(), llvm_send_return_value_ptr);

		// Set the step in the frame.
		llvm::ConstantInt* llvm_yield_idx =
			llvm::ConstantInt::get(*context, llvm::APInt(32, yield_idx + 1, true));
		builder->CreateStore(llvm_yield_idx, llvm_step_value_ptr);

		// Copy all vars back into the frame
		for( auto [sym, alloca_addr] : vars )
		{
			int frame_idx = frame.sym_frame_idx_lut.at(sym);
			llvm::Value* llvm_gep =
				builder->CreateStructGEP(llvm_frame_type, llvm_frame_arg, frame_idx);

			codegen_memcpy(*alloca_addr, llvm_gep);
		}

		builder->CreateRetVoid();
	}

	return Expr::Empty();
}

Codegen::codegen_async_step_t
Codegen::codegen_async_step(HirNode* hir_func, codegen_async_frame_t frame)
{
	llvm::Type* llvm_frame_ty = frame.frame_ty;

	HirFunc& func_nod = hir_cast<HirFunc>(hir_func);
	HirNode* hir_proto = func_nod.proto;

	HirFuncProto& proto = hir_cast<HirFuncProto>(hir_proto);
	SymType& proto_impl_sym = sym_cast<SymType>(proto.impl_sym);

	SymType& send_ty_sym = sym_cast<SymType>(sym_unalias(proto_impl_sym.scope.find("Send")));

	llvm::Type* llvm_send_ty = get_type(send_ty_sym.ty);
	llvm::Type* llvm_send_opt_ty = llvm::StructType::create(
		*context, {llvm::Type::getInt1Ty(*context), llvm_send_ty->getPointerTo()}, "SendOpt");

	SymType& iter_ty_sym = sym_cast<SymType>(sym_unalias(proto_impl_sym.scope.find("Iter")));
	llvm::Type* llvm_iter_ty = get_type(iter_ty_sym.ty);
	Sym* send_sym = proto_impl_sym.scope.find("send");
	SymFunc& send_sym_func = sym_cast<SymFunc>(send_sym);
	TyFunc const& send_ty_func = ty_cast<TyFunc>(send_sym_func.ty);

	llvm::Type* llvm_iter_ret_ty = get_type(send_ty_func.rt_qty.ty);

	llvm::Type* llvm_void_ty = llvm::Type::getVoidTy(*context);

	std::vector<Arg> args;
	args.push_back(Arg(nullptr, llvm_iter_ret_ty, true, false));
	args.push_back(Arg(nullptr, llvm_frame_ty, false, true));
	args.push_back(Arg(nullptr, llvm_send_opt_ty, false, true));

	// TODO: Function that takes
	// 1. sret of llvm_iter_res_ty
	// 2. implicit byval of llvm_frame_ty
	// 3. byval of llvm send_opt_ty

	auto llvm_args = to_llvm_arg_tys(args);
	llvm::FunctionType* llvm_fn_ty = llvm::FunctionType::get(
		llvm_void_ty, llvm_args, proto.var_arg == HirFuncProto::VarArg::VarArg);

	llvm::Function* llvm_fn = llvm::Function::Create(
		llvm_fn_ty, linkage(proto.linkage), func_name(proto.sym) + "_step", mod.get());

	// We use a function on the stack here because there is no sym for the step function
	// you cannot reference it.
	Function func = Function::FromArgs(llvm_fn, args);
	func.llvm_send_ty = llvm_send_ty;
	func.llvm_send_opt_ty = llvm_send_opt_ty;
	current_func = &func;

	llvm::BasicBlock* llvm_entry_bb = llvm::BasicBlock::Create(*context, "entry", llvm_fn);
	builder->SetInsertPoint(llvm_entry_bb);

	// 1. Function entry (in this case, that's just the implicit send arguments)
	// 2. Create the rehydration block
	// 	This occurs before the jump table.
	// 3. Create the jump table
	//  Keep reference to it so yield statements can add themselves.
	// 4. Body (this is actually generated first.)
	llvm::Argument* llvm_frame_arg = llvm_fn->getArg(1);
	for( auto& [sym, frame_idx] : frame.sym_frame_idx_lut )
	{
		llvm::Value* llvm_gep = builder->CreateStructGEP(frame.frame_ty, llvm_frame_arg, frame_idx);

		vars.add(sym, Address(llvm_gep, llvm_gep->getType()->getPointerElementType()));
	}

	// Insert a body bb after entry bb, this will be the first block jumped to
	llvm::BasicBlock* llvm_body_bb = llvm::BasicBlock::Create(*context, "body", llvm_fn);
	builder->SetInsertPoint(llvm_body_bb);

	codegen_block(func_nod.body);

	// Now backpatch the async stuff.
	// Insert the rehydration step before the entry step.
	// codegen_async_step_rehydration(hir_func, llvm_entry_bb, frame);
	// llvm_body_bb is the 0th block of the jump table.
	codegen_async_step_jump_table(hir_func, llvm_entry_bb, llvm_body_bb, frame);

	codegen_async_step_t step = {
		.send_ty = llvm_send_ty,
		.send_opt_ty = llvm_send_opt_ty,
		.iter_ty = llvm_iter_ty,
		.iter_ret_ty = llvm_iter_ret_ty,
		.step_fn = llvm_fn};
	codegen_async_step_suspends_backpatch(hir_func, step, frame);

	vars.clear();
	current_func = nullptr;

	return step;
}

Expr
Codegen::codegen_async_begin(
	HirNode* hir_proto, llvm::Type* llvm_frame_ty, codegen_async_step_t step)
{
	llvm::Function* llvm_step_fn = step.step_fn;
	llvm::Type* llvm_send_opt_ty = step.send_opt_ty;
	llvm::Type* llvm_send_ty = step.send_ty;
	llvm::Type* llvm_iter_ret_ty = step.iter_ret_ty;

	std::cout << "Frame Ptr: " << std::hex << llvm_frame_ty << std::endl;
	std::cout << "Iter Ret Ptr: " << std::hex << llvm_iter_ret_ty << std::endl;
	std::cout << "Send Opt Ptr: " << std::hex << llvm_send_opt_ty << std::endl;

	HirFuncProto& proto = hir_cast<HirFuncProto>(hir_proto);

	std::vector<Arg> args;
	args.push_back(Arg(nullptr, llvm_iter_ret_ty, true, false));
	args.push_back(Arg(nullptr, llvm_frame_ty, false, true));

	llvm::FunctionType* llvm_begin_fn_ty =
		llvm::FunctionType::get(llvm::Type::getVoidTy(*context), to_llvm_arg_tys(args), false);

	llvm::Function* llvm_begin_fn = llvm::Function::Create(
		llvm_begin_fn_ty, linkage(proto.linkage), func_name(proto.sym), mod.get());

	// Get the "begin" symbol
	SymType& proto_impl_sym = sym_cast<SymType>(proto.impl_sym);
	Sym* begin_sym = proto_impl_sym.scope.find("begin");
	assert(begin_sym);
	auto emplaced = funcs.emplace(begin_sym, Function::FromArgs(llvm_begin_fn, args));
	Function* func = &emplaced.first->second;

	llvm::BasicBlock* llvm_entry_bb = llvm::BasicBlock::Create(*context, "entry", llvm_begin_fn);
	builder->SetInsertPoint(llvm_entry_bb);

	llvm::Value* llvm_send_opt_struct_val = builder->CreateAlloca(llvm_send_opt_ty);

	// The optional flag whether it's null is the 0th entry in the struct.
	llvm::Value* llvm_send_opt_flag =
		builder->CreateStructGEP(llvm_send_opt_ty, llvm_send_opt_struct_val, 0);

	llvm::ConstantInt* llvm_zero_1bit = llvm::ConstantInt::get(*context, llvm::APInt(1, 0, true));

	builder->CreateStore(llvm_zero_1bit, llvm_send_opt_flag);

	llvm::Value* llvm_iter_ret_ptr = llvm_begin_fn->getArg(0);
	llvm::Value* llvm_frame_ptr = llvm_begin_fn->getArg(1);
	builder->CreateCall(
		llvm_step_fn,
		// Step Return Type
		// Frame as first arg
		// Send Optional Type
		// Note! Byval and sret args don't need to be allocad
		{llvm_iter_ret_ptr, llvm_frame_ptr, llvm_send_opt_struct_val});
	builder->CreateRetVoid();

	vars.clear();
	current_func = nullptr;

	return Expr::Empty();
}

Expr
Codegen::codegen_async_send(
	HirNode* hir_proto, llvm::Type* llvm_frame_ty, codegen_async_step_t step)
{
	llvm::Function* llvm_step_fn = step.step_fn;
	llvm::Type* llvm_send_opt_ty = step.send_opt_ty;
	llvm::Type* llvm_send_ty = step.send_ty;
	llvm::Type* llvm_iter_ret_ty = step.iter_ret_ty;

	HirFuncProto& proto = hir_cast<HirFuncProto>(hir_proto);

	std::vector<Arg> args;
	args.push_back(Arg(nullptr, llvm_iter_ret_ty, true, false));
	args.push_back(Arg(nullptr, llvm_frame_ty, false, true));
	// TODO: Use qty.is_aggregate_ty()
	args.push_back(Arg(nullptr, llvm_send_ty, false, llvm_send_ty->isAggregateType()));

	llvm::FunctionType* llvm_send_fn_ty =
		llvm::FunctionType::get(llvm::Type::getVoidTy(*context), to_llvm_arg_tys(args), false);

	llvm::Function* llvm_send_fn = llvm::Function::Create(
		llvm_send_fn_ty, linkage(proto.linkage), func_name(proto.sym), mod.get());

	// Get the "send" symbol
	SymType& proto_impl_sym = sym_cast<SymType>(proto.impl_sym);
	Sym* send_sym = proto_impl_sym.scope.find("send");
	assert(send_sym);
	auto emplaced = funcs.emplace(send_sym, Function::FromArgs(llvm_send_fn, args));
	Function* func = &emplaced.first->second;

	llvm::BasicBlock* llvm_entry_bb = llvm::BasicBlock::Create(*context, "entry", llvm_send_fn);
	builder->SetInsertPoint(llvm_entry_bb);
	codegen_func_entry(func);

	llvm::Value* llvm_send_opt_struct_val = builder->CreateAlloca(llvm_send_opt_ty);

	// The optional flag whether it's null is the 0th entry in the struct.
	llvm::Value* llvm_send_opt_flag =
		builder->CreateStructGEP(llvm_send_opt_ty, llvm_send_opt_struct_val, 0);

	llvm::ConstantInt* llvm_one_1bit = llvm::ConstantInt::get(*context, llvm::APInt(1, 1, true));

	builder->CreateStore(llvm_one_1bit, llvm_send_opt_flag);

	// TODO: Store a pointer to the arg in the struct.

	builder->CreateCall(
		llvm_step_fn,
		// Step Return Type
		// Frame as first arg
		// Send Optional Type
		// Note! Byval and sret args don't need to be allocad
		{llvm_send_fn->getArg(0), llvm_send_fn->getArg(1), llvm_send_opt_struct_val});
	builder->CreateRetVoid();

	vars.clear();
	current_func = nullptr;

	return Expr::Empty();
}

Expr
Codegen::codegen_async_close(HirNode* hir_proto, llvm::Type* frame)
{
	HirFuncProto& proto = hir_cast<HirFuncProto>(hir_proto);

	// Get the "close" symbol
	SymType& proto_impl_sym = sym_cast<SymType>(proto.impl_sym);
	Sym* close_sym = proto_impl_sym.scope.find("close");
	assert(close_sym);

	return Expr::Empty();
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
	case HirFuncCall::CallKind::PtrCall:
		return codegen_func_call_indirect(hir_call, sret);
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

			codegen_memcpy(expr.address(), val);

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

Expr
Codegen::codegen_func_call_indirect(HirNode* hir_call, Expr sret)
{
	HirFuncCall& call_nod = hir_cast<HirFuncCall>(hir_call);
	assert(call_nod.kind == HirFuncCall::CallKind::PtrCall);

	Expr this_expr = codegen_expr(call_nod.this_expr);
	Function& callee = funcs.at(call_nod.callee);

	std::cout << "Sret Param Type: " << std::hex << callee.sret.address().llvm_allocated_type()
			  << std::endl;
	std::cout << "Sret Arg Type: " << std::hex << sret.address().llvm_allocated_type() << std::endl;

	std::vector<llvm::Value*> llvm_arg_values;
	if( !callee.sret.is_void() )
	{
		// Allocate the sret arg if one is not provided.
		llvm::Value* val;
		if( sret.is_address() )
			val = sret.address().llvm_pointer();
		else
			val = builder->CreateAlloca(callee.sret.address().llvm_allocated_type());

		llvm_arg_values.push_back(val);
	}

	// llvm::Value* llvm_this_val = codegen_eval(this_expr);
	llvm_arg_values.push_back(this_expr.address().llvm_pointer());

	for( int i = 0; i < callee.ir_arg_count(); i++ )
	{
		HirNode* hir_expr = call_nod.args.at(i);
		Arg& arg = callee.ir_arg(i);
		Expr expr = codegen_expr(hir_expr);

		if( arg.is_byval() )
		{
			llvm::Value* val = builder->CreateAlloca(arg.type);

			codegen_memcpy(expr.address(), val);

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
		return RValue(builder->CreateSExtOrTrunc(value, dest));
	case TyInt::Sign::Unsigned:
		return RValue(builder->CreateSExtOrTrunc(value, dest));
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

class InsertEntryBlock
{
	llvm::BasicBlock* llvm_restore_bb;
	llvm::IRBuilder<>* builder;

public:
	InsertEntryBlock(llvm::IRBuilder<>* builder, llvm::Function* llvm_fn)
		: builder(builder)
	{
		llvm_restore_bb = builder->GetInsertBlock();
		builder->SetInsertPoint(&llvm_fn->getEntryBlock());
	}

	~InsertEntryBlock() { builder->SetInsertPoint(llvm_restore_bb); }
};

Expr
Codegen::codegen_let(HirNode* hir_let)
{
	HirLet& let = hir_cast<HirLet>(hir_let);

	llvm::Type* type = get_type(var_ty(let.sym));
	std::cout << "Var Type: " << std::hex << type << std::endl;

	{
		InsertEntryBlock block(builder.get(), current_func->llvm_func);

		// TODO: Arrays.
		llvm::Value* val = builder->CreateAlloca(type);
		vars.add(let.sym, Address(val, type));
	}

	return Expr::Empty();
}

Expr
Codegen::codegen_switch(HirNode* hir_switch)
{
	HirSwitch& sw = hir_cast<HirSwitch>(hir_switch);

	Expr expr = codegen_expr(sw.cond);

	llvm::Function* llvm_fn = current_func->llvm_func;
	llvm::BasicBlock* llvm_merge_bb = llvm::BasicBlock::Create(*context, "");

	llvm::SwitchInst* llvm_sw = builder->CreateSwitch(codegen_eval(expr), llvm_merge_bb);

	for( auto branch : sw.branches )
	{
		int jt_val = branch.value;
		llvm::ConstantInt* llvm_const_int =
			llvm::ConstantInt::get(*context, llvm::APInt(32, jt_val, true));

		llvm::BasicBlock* llvm_case_bb = llvm::BasicBlock::Create(*context, "", llvm_fn);
		builder->SetInsertPoint(llvm_case_bb);
		Expr case_body = codegen_expr(branch.then);
		builder->CreateBr(llvm_merge_bb);

		llvm_sw->addCase(llvm_const_int, llvm_case_bb);
	}

	builder->SetInsertPoint(llvm_merge_bb);
	llvm_merge_bb->insertInto(llvm_fn);

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

/**
 * This renders if-elsif-else
 *
 * if (a) {
 * 	...
 * } else if (b) {
 * 	...
 * } else {
 * 	...
 * }
 *
 * @param hir_if
 * @return Expr
 */
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

	Address* iter_vars = vars.find(var.sym);
	assert(iter_vars);

	return Expr(Address(*iter_vars));
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
	case HirNodeKind::Switch:
		return codegen_switch(hir_expr);
	case HirNodeKind::Yield:
		return codegen_yield(hir_expr);
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
		codegen_memcpy(expr.address(), dest);

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
Codegen::codegen_yield(HirNode* hir_yield)
{
	HirYield& yield_nod = hir_cast<HirYield>(hir_yield);

	// The suspend block is the current bb.
	Expr yield_expr = codegen_expr(yield_nod.expr);

	// The suspend block gets backpatched.
	llvm::BasicBlock* llvm_suspend_bb =
		llvm::BasicBlock::Create(*context, "suspend", current_func->llvm_func);
	builder->CreateBr(llvm_suspend_bb);

	llvm::BasicBlock* llvm_resume_bb =
		llvm::BasicBlock::Create(*context, "resume", current_func->llvm_func);

	current_func->yield_points.push_back(
		FunctionYieldPoint(llvm_suspend_bb, llvm_resume_bb, yield_expr));

	/**
	 * Exit from the suspend block will be codegened later.
	 */

	builder->SetInsertPoint(llvm_resume_bb);

	// TODO: Check that the value is present, crash otherwise.
	llvm::Value* llvm_send_payload_ptr = builder->CreateStructGEP(
		current_func->llvm_send_opt_ty, current_func->llvm_func->getArg(2), 1);

	// The value stored in the send_opt_ty is actually a pointer to the value,
	// since we're taking a gep to that value, the result is pointer->pointer->opt_ty

	llvm::Value* llvm_send_payload =
		builder->CreateLoad(current_func->llvm_send_ty->getPointerTo(), llvm_send_payload_ptr);

	return Expr(Address(llvm_send_payload, current_func->llvm_send_ty));
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
Codegen::codegen_memcpy(Address src_address, llvm::Value* dest)
{
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
	return tys.at(ty);
}