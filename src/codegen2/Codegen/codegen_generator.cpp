#include "codegen_generator.h"

#include "../Codegen.h"
#include "LLVMFnSigInfoBuilder.h"
#include "cg_copy.h"
#include "codegen_fn_sig_info.h"
#include "codegen_function.h"
#include "lookup.h"
#include "operand.h"

#include <utility>

using namespace cg;

// Frame
// resume
//

static llvm::StructType*
cg_send_result_struct(CG& codegen, LLVMAsyncFn& async_fn)
{
	// // Create a send result struct.
	//
	// struct SendResultStruct<TRet> {
	//     bool done;
	//     TRet value;
	// }
	//
	std::vector<llvm::Type*> members;
	llvm::Type* llvm_done_ty = llvm::Type::getInt1Ty(*codegen.Context);
	members.push_back(llvm_done_ty);

	// TODO: Hardcode to i32
	llvm::Type* llvm_yield_ty = llvm::Type::getInt32Ty(*codegen.Context);
	members.push_back(llvm_yield_ty);

	llvm::StructType* llvm_struct_type =
		llvm::StructType::create(*codegen.Context, members, "SendResult<i32>");

	async_fn.llvm_send_return_type = llvm_struct_type;

	return llvm_struct_type;
}

static void
cg_send_fn(
	CG& codegen,
	llvm::Function* llvm_fn,
	llvm::StructType* llvm_send_return_type,
	llvm::StructType* llvm_frame_type,
	llvm::BasicBlock* llvm_entry_block)
{
	// Accept send type, which is void for now,
	// Frame struct is first arg.
	// Rehydrate allocas,
	// Jump
	// llvm::FunctionType* llvm_fn_ty = llvm::FunctionType::get(
	// 	llvm::Type::getVoidTy(*codegen.Context),
	// 	{
	// 		llvm_send_return_type,
	// 		llvm_frame_type->getPointerTo(),
	// 	},
	// 	false);

	// llvm::Function* llvm_fn = llvm::Function::Create(
	// 	llvm_fn_ty, llvm::Function::ExternalLinkage, "Send", codegen.Module.get());

	codegen.Builder->SetInsertPoint(codegen.async_context.value().entry_block);

	// Deref first element of from frame.
	llvm::Value* llvm_step_value_ptr =
		codegen.Builder->CreateStructGEP(llvm_frame_type, llvm_fn->getArg(1), 0);

	llvm::Value* llvm_step_value =
		codegen.Builder->CreateLoad(llvm::Type::getInt32Ty(*codegen.Context), llvm_step_value_ptr);

	// Basic Block for when we send after the coro is done.
	llvm::BasicBlock* llvm_bad_send_bb = llvm::BasicBlock::Create(*codegen.Context, "BadSend");

	llvm::SwitchInst* llvm_switch =
		codegen.Builder->CreateSwitch(llvm_step_value, llvm_bad_send_bb, 4);

	llvm::ConstantInt* llvm_zero =
		llvm::ConstantInt::get(*codegen.Context, llvm::APInt(32, 0, true));
	llvm_switch->addCase(llvm_zero, llvm_entry_block);

	// LLVMFnArgInfo& frame_arg = fn.frame_arg.value();
	// LLVMAddress frame_addr = frame_arg.lvalue.address();

	LLVMAsyncFn& async_fn = codegen.async_context.value();

	// Rehydrate in the resume blocks
	int jump_idx = 1; // 0 is the entry block above.

	for( LLVMYieldPoint& yield : async_fn.yield_bbs )
	{
		llvm::BasicBlock* llvm_before_resume = llvm::BasicBlock::Create(
			*codegen.Context, "BeforeResume", llvm_fn, yield.llvm_resume_block);
		codegen.Builder->SetInsertPoint(llvm_before_resume);
		int alloca_idx = 1;
		for( LLVMAddress& stack_address : async_fn.allocas )
		{
			auto llvm_gep =
				codegen.Builder->CreateStructGEP(llvm_frame_type, llvm_fn->getArg(1), alloca_idx);
			LLVMAddress frame_address(llvm_gep, stack_address.llvm_allocated_type());
			cg_copy(codegen, frame_address, stack_address);
			alloca_idx += 1;
		}

		codegen.Builder->CreateBr(yield.llvm_resume_block);

		llvm::ConstantInt* llvm_idx =
			llvm::ConstantInt::get(*codegen.Context, llvm::APInt(32, jump_idx, true));

		llvm_switch->addCase(llvm_idx, llvm_before_resume);
		jump_idx += 1;
	}

	llvm::ConstantInt* llvm_zero_1bit =
		llvm::ConstantInt::get(*codegen.Context, llvm::APInt(1, 0, true));
	llvm::ConstantInt* llvm_one_1bit =
		llvm::ConstantInt::get(*codegen.Context, llvm::APInt(1, 1, true));

	// Store the variables in the frame in the suspend blocks
	int yield_idx = 1;
	for( LLVMYieldPoint& yield : async_fn.yield_bbs )
	{
		llvm::BasicBlock* llvm_suspend_block = yield.llvm_suspend_block;
		codegen.Builder->SetInsertPoint(llvm_suspend_block);

		llvm::ConstantInt* llvm_idx =
			llvm::ConstantInt::get(*codegen.Context, llvm::APInt(32, yield_idx, true));
		auto llvm_step_gep =
			codegen.Builder->CreateStructGEP(llvm_frame_type, llvm_fn->getArg(1), 0);
		codegen.Builder->CreateStore(llvm_idx, llvm_step_gep);

		int alloca_idx = 1;
		for( LLVMAddress& stack_address : async_fn.allocas )
		{
			auto llvm_gep =
				codegen.Builder->CreateStructGEP(llvm_frame_type, llvm_fn->getArg(1), alloca_idx);
			LLVMAddress frame_address(llvm_gep, stack_address.llvm_allocated_type());
			cg_copy(codegen, stack_address, frame_address);
			alloca_idx += 1;
		}

		auto llvm_ret_done_ptr =
			codegen.Builder->CreateStructGEP(llvm_send_return_type, llvm_fn->getArg(0), 0);
		codegen.Builder->CreateStore(llvm_zero_1bit, llvm_ret_done_ptr);

		auto llvm_yield_ptr =
			codegen.Builder->CreateStructGEP(llvm_send_return_type, llvm_fn->getArg(0), 1);
		if( yield.yield_expr.is_rvalue() )
		{
			auto llvm_yielded_value = codegen_operand_expr(codegen, yield.yield_expr);
			// TODO: Support complex return type.
			codegen.Builder->CreateStore(llvm_yielded_value, llvm_yield_ptr);
		}
		else
		{
			LLVMAddress src_addr = yield.yield_expr.address();
			LLVMAddress dest_addr =
				LLVMAddress(llvm_yield_ptr, llvm_yield_ptr->getType()->getPointerElementType());
			cg_copy(codegen, src_addr, dest_addr);
		}

		codegen.Builder->CreateRetVoid();
	}

	for( LLVMYieldPoint& return_block : async_fn.early_return_blocks )
	{
		codegen.Builder->SetInsertPoint(return_block.llvm_suspend_block);
		llvm::ConstantInt* llvm_bad_resume_idx =
			llvm::ConstantInt::get(*codegen.Context, llvm::APInt(32, -1, true));
		auto llvm_step_gep =
			codegen.Builder->CreateStructGEP(llvm_frame_type, llvm_fn->getArg(1), 0);
		codegen.Builder->CreateStore(llvm_bad_resume_idx, llvm_step_gep);

		auto llvm_ret_done_ptr =
			codegen.Builder->CreateStructGEP(llvm_send_return_type, llvm_fn->getArg(0), 0);
		codegen.Builder->CreateStore(llvm_one_1bit, llvm_ret_done_ptr);

		auto llvm_return_ptr =
			codegen.Builder->CreateStructGEP(llvm_frame_type, llvm_fn->getArg(1), 2);

		// TODO: Store return expr in frame.

		if( return_block.yield_expr.is_rvalue() )
		{
			auto llvm_yielded_value = codegen_operand_expr(codegen, return_block.yield_expr);
			// TODO: Support complex return type.
			codegen.Builder->CreateStore(llvm_yielded_value, llvm_return_ptr);
		}
		else
		{
			LLVMAddress src_addr = return_block.yield_expr.address();
			LLVMAddress result_addr =
				LLVMAddress(llvm_return_ptr, llvm_return_ptr->getType()->getPointerElementType());
			cg_copy(codegen, src_addr, result_addr);
		}

		codegen.Builder->CreateBr(return_block.llvm_resume_block);
	}

	codegen.Builder->SetInsertPoint(llvm_bad_send_bb);
	codegen.Builder->CreateRetVoid();

	llvm_fn->getBasicBlockList().push_back(llvm_bad_send_bb);
}

static void
cg_send_call(CG& codegen, cg::LLVMFnInfo& fn)
{
	// Call send function and pass frame
}

// static llvm::StructType*
// begin_frame_struct()
// {
// 	// Problems, the struct needs to be known before the function is created.
// 	// How to generate the yield statements to rehydrate the locals
// 	// without the frame struct?

// 	llvm::StructType* llvm_struct_type =
// 		llvm::StructType::create(*codegen.Context, {}, "AsyncFn<i32, i32>");
// 	llvm::Type* llvm_step_ty = llvm::Type::getInt32Ty(*codegen.Context);
// }

static llvm::StructType*
cg_frame(CG& codegen, ir::GeneratorFn& fn, LLVMAsyncFn& async_fn)
{
	// // Codegen Frame
	//
	// struct Frame<TRet> {
	//     i32 step;
	//     ...Locals value;
	// }
	//
	std::vector<llvm::Type*> members;

	llvm::Type* llvm_step_idx_type = llvm::Type::getInt32Ty(*codegen.Context);
	members.push_back(llvm_step_idx_type);

	for( auto& local : fn.locals )
	{
		llvm::Type* llvm_local_type = get_type(codegen, local.type()).unwrap();
		members.push_back(llvm_local_type);
	}

	// TODO: Hardcode to i32
	llvm::Type* llvm_ret_ty = llvm::Type::getInt32Ty(*codegen.Context);
	members.push_back(llvm_ret_ty);

	llvm::StructType* llvm_struct_type =
		llvm::StructType::create(*codegen.Context, members, "AsyncFn<i32, i32>");

	// LLVMFrameArg frame;
	// frame.frame_type = llvm_struct_type;
	// frame.locals = members;

	async_fn.llvm_frame_type = llvm_struct_type;

	return llvm_struct_type;
}

CGResult<CGExpr>
cg::codegen_generator(CG& codegen, ir::IRGenerator* ir_gen)
{
	// Begin the frame struct. The members will be populated after codegen
	//
	codegen.async_context = LLVMAsyncFn();
	llvm::StructType* llvm_send_return_type =
		cg_send_result_struct(codegen, codegen.async_context.value());

	codegen.types.emplace(
		ir_gen->send_fn_name_ref.type().type->get_return_type().value().type,
		llvm_send_return_type);
	// LLVMFnSigInfoBuilder builder(name, ir_proto->fn_type);

	llvm::StructType* llvm_frame_type =
		cg_frame(codegen, ir_gen->fn, codegen.async_context.value());

	codegen.types.emplace(ir_gen->proto->rt->type_instance.type, llvm_frame_type);

	llvm::FunctionType* llvm_fn_ty = llvm::FunctionType::get(
		llvm::Type::getVoidTy(*codegen.Context),
		{
			llvm_send_return_type->getPointerTo(),
			llvm_frame_type->getPointerTo(),
		},
		false);

	// llvm::Function* llvm_fn = llvm::Function::Create(
	// 	llvm_fn_ty, llvm::Function::ExternalLinkage, "Send", codegen.Module.get());
	// llvm::Argument* llvm_arg = llvm_fn->getArg(0);
	// llvm_arg->addAttrs(llvm::AttrBuilder().addStructRetAttr(llvm_send_return_type));

	// No arguments for now.

	// Generate code as normal
	// for( auto [name_id, arg] : ctx.named_args )
	// 	cg.values.insert_or_assign(name_id, arg.lvalue);

	LLVMFnSigInfoBuilder send_sig_info_builder(
		ir_gen->send_fn_name_ref, ir_gen->send_fn_name_ref.type().type);
	send_sig_info_builder.llvm_ret_ty = llvm::Type::getVoidTy(*codegen.Context);
	send_sig_info_builder.add_arg_type(
		LLVMArgABIInfo(LLVMArgABIInfo::Kind::SRet, llvm_send_return_type->getPointerTo()));
	send_sig_info_builder.add_arg_type(
		ir_gen->send_fn_name_ref.lookup("frame").value(),
		LLVMArgABIInfo(LLVMArgABIInfo::Kind::Value, llvm_frame_type->getPointerTo()));

	LLVMFnSigInfo send_fn_sig = codegen_fn_sig_info(codegen, send_sig_info_builder);
	codegen.add_function(ir_gen->send_fn_name_ref.type().type, send_fn_sig);

	llvm::Function* llvm_fn = send_fn_sig.llvm_fn;

	//
	// Create the basic blocks, then once code is generated, we have all the info we need
	// Create the function and add the basic block.

	LLVMFnInfo send_fn = codegen_function_entry(codegen, send_fn_sig).unwrap();

	llvm::BasicBlock* llvm_entry_block = &llvm_fn->getEntryBlock();

	llvm::BasicBlock* llvm_jump_block =
		llvm::BasicBlock::Create(*codegen.Context, "Jump", llvm_fn, llvm_entry_block);
	codegen.async_context.value().entry_block = llvm_jump_block;

	codegen.codegen_block(send_fn, ir_gen->block);

	// Generate frame and function

	cg_send_fn(codegen, llvm_fn, llvm_send_return_type, llvm_frame_type, llvm_entry_block);

	// llvm::FunctionType* llvm_init_fn_ty = llvm::FunctionType::get(
	// 	llvm::Type::getVoidTy(*codegen.Context),
	// 	{
	// 		llvm_frame_type->getPointerTo(),
	// 	},
	// 	false);

	// llvm::Function* llvm_init_fn = llvm::Function::Create(
	// 	llvm_fn_ty, llvm::Function::ExternalLinkage, "Init", codegen.Module.get());

	LLVMFnSigInfoBuilder sig_info_builder(ir_gen->proto->name, ir_gen->proto->fn_type);
	sig_info_builder.llvm_ret_ty = llvm::Type::getVoidTy(*codegen.Context);
	sig_info_builder.add_arg_type(
		LLVMArgABIInfo(LLVMArgABIInfo::Kind::SRet, llvm_frame_type->getPointerTo()));

	LLVMFnSigInfo init_fn = codegen_fn_sig_info(codegen, sig_info_builder);
	codegen.add_function(ir_gen->proto->fn_type, init_fn);
	llvm::BasicBlock* llvm_init_entry_block =
		llvm::BasicBlock::Create(*codegen.Context, "entry", init_fn.llvm_fn);

	codegen.Builder->SetInsertPoint(llvm_init_entry_block);

	LLVMAddress frame_arg(init_fn.llvm_fn->getArg(0), llvm_frame_type);
	cg_zero(codegen, frame_arg);

	codegen.Builder->CreateRetVoid();

	codegen.async_context.reset();

	return CGExpr();
}

CGResult<CGExpr>
cg::codegen_yield(CG& codegen, cg::LLVMFnInfo& fn, ir::IRYield* ir_yield)
{
	// The "generator" function is the "Send" function. So create the
	// The generator call creates the "Send" function
	//
	// # Create a SUSPENDING BB and save it until the frame generation
	// In frame generation
	// Loop over allocas that were stored from generation.
	// Store allocas in frame.
	// Set step index in the frame to the index of this suspending bb.
	// return yield type (in struct with false... {TRet, false})
	//
	// # Create a RESUMING BB
	// Then return the "Send" argument, e.g. the function Send(<my_arg>)
	// as the CGExpr.
	//
	//
	// After the function finishes codegen,
	// Create the frame struct
	// create the Function that takes the frame type and the send type (we are in it now...
	// actually)
	// Create the rehydration of the local vars and args (which we kept track of) Create the jump
	// table to the RESUMING BBs. Add the RESUMING BBs to the jump table.
	//
	//
	// # Early returns
	// Set the step the end bb
	// return yield type (in struct with true... {TRet, true})
	//
	//
	// # Coro Constructor
	// after frame generation, generator a constructor function
	// that fills the frame argument fields with the function args

	// TODO: Need gotos until better flow control for dtors or defer.

	// The suspend block is the current bb.
	auto maybe_yield_expr = codegen.codegen_expr(fn, ir_yield->expr);
	auto yield_expr = maybe_yield_expr.unwrap();

	llvm::BasicBlock* llvm_suspend_bb = codegen.Builder->GetInsertBlock();

	llvm::BasicBlock* llvm_resume_bb =
		llvm::BasicBlock::Create(*codegen.Context, "resume", fn.sig_info.llvm_fn);

	auto& async_ctx = codegen.async_context.value();
	async_ctx.add_yield(LLVMYieldPoint(llvm_suspend_bb, llvm_resume_bb, yield_expr));

	// We will insert the rehydration before this block
	codegen.Builder->SetInsertPoint(llvm_resume_bb);

	// TODO: Return send argument.
	return CGExpr();
}