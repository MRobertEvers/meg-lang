#include "codegen_generator.h"

#include "../Codegen.h"
#include "lookup.h"

#include <utility>

using namespace cg;

// Frame
// resume
//

static void
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
	llvm::Type* llvm_payload_ty = llvm::Type::getInt32Ty(*codegen.Context);
	members.push_back(llvm_payload_ty);

	llvm::StructType* llvm_struct_type =
		llvm::StructType::create(*codegen.Context, members, "SendResult<i32>");

	async_fn.llvm_send_return_type = llvm_struct_type;
}

static void
cg_send_fn(CG& codegen, cg::LLVMFnInfo& fn)
{
	// Accept send type, which is void for now,
	// Frame struct is first arg.
	// Rehydrate allocas,
	// Jump
	llvm::BasicBlock* llvm_entry_bb = llvm::BasicBlock::Create(*codegen.Context, "AsyncEnter");
	codegen.Builder->SetInsertPoint(llvm_entry_bb);

	LLVMFnArgInfo& frame_arg = fn.frame_arg.value();
	LLVMAddress frame_addr = frame_arg.lvalue.address();

	// Rehydrate

	// Deref first element of from frame.
	llvm::Value* llvm_step_value = codegen.Builder->CreateStructGEP(
		frame_addr.llvm_allocated_type(), frame_addr.llvm_pointer(), 0);

	// Basic Block for when we send after the coro is done.
	llvm::BasicBlock* llvm_bad_send_bb = llvm::BasicBlock::Create(*codegen.Context, "BadSend");

	llvm::SwitchInst* llvm_switch =
		codegen.Builder->CreateSwitch(llvm_step_value, llvm_bad_send_bb, 4);
}

static void
cg_send_call(CG& codegen, cg::LLVMFnInfo& fn)
{
	// Call send function and pass frame
}

static llvm::StructType*
begin_frame_struct()
{
	// Problems, the struct needs to be known before the function is created.
	// How to generate the yield statements to rehydrate the locals
	// without the frame struct?

	llvm::StructType* llvm_struct_type =
		llvm::StructType::create(*codegen.Context, {}, "AsyncFn<i32, i32>");
	llvm::Type* llvm_step_ty = llvm::Type::getInt32Ty(*codegen.Context);
}

static void
cg_frame(CG& codegen, LLVMAsyncFn& async_fn)
{
	// // Codegen Frame
	//
	// struct SendResultStruct<TRet> {
	//     bool done;
	//     TRet value;
	// }
	//
	std::vector<llvm::Type*> members;

	for( auto& addr : async_fn.allocas )
		members.push_back(addr.llvm_allocated_type());

	llvm::StructType* llvm_struct_type =
		llvm::StructType::create(*codegen.Context, members, "AsyncFn<i32, i32>");

	LLVMFrameArg frame;
	frame.frame_type = llvm_struct_type;
	frame.locals = members;

	async_fn.llvm_frame_type = frame;
}

CGResult<CGExpr>
cg::codegen_generator(CG& codegen, ir::IRGenerator* ir_gen)
{
	// Begin the frame struct. The members will be populated after codegen
	//
	codegen.async_context = LLVMAsyncFn();
	cg_send_result_struct(codegen, codegen.async_context.value());

	// LLVMFnSigInfoBuilder builder(name, ir_proto->fn_type);

	//
	// Create the basic blocks, then once code is generated, we have all the info we need
	// Create the function and add the basic block.
	llvm::BasicBlock* llvm_entry_bb = llvm::BasicBlock::Create(*codegen.Context, "entry");
	codegen.Builder->SetInsertPoint(llvm_entry_bb);

	// No arguments for now.

	// Generate code as normal
	// for( auto [name_id, arg] : ctx.named_args )
	// 	cg.values.insert_or_assign(name_id, arg.lvalue);

	codegen.codegen_block(ctx, block);

	// Generate frame and function
	cg_frame(codegen, codegen.async_context.value());
	cg_send_fn(codegen, codegen.async_context.value());
}

CGResult<CGExpr>
cg::codegen_yield(CG& codegen, cg::LLVMFnInfo&, ir::IRYield* ir_yield)
{
	// Create a SUSPENDING BB and save it until the frame generation
	// In frame generation
	// Store allocas
	// return yield type.
	//
	// Create a RESUMING BB
	// Then return the "Send" argument, e.g. the function Send(<my_arg>)
	// as the CGExpr.
	//
	//
	// After the function finishes codegen,
	// Create the frame struct
	// create the Function that takes the frame type and the send type
	// Create the rehydration of the local vars (which we kept track of)
	// Create the jump table
	// Add the RESUMING BBs to the jump table.
	//
}