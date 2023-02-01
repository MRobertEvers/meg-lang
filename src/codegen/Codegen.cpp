#include "Codegen.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

using namespace llvm;
using namespace codegen;
using namespace ast;

static int
get_element_index_in_struct(ast::Struct const* st, std::string const& name)
{
	unsigned int idx = 0;
	for( auto& mem : st->MemberVariables )
	{
		if( mem->Name->get_name() == name )
		{
			return idx;
		}
		idx++;
	}

	return -1;
}

static ast::MemberVariableDeclaration*
get_member_of_struct(ast::Struct const* st, std::string const& name)
{
	for( auto& mem : st->MemberVariables )
	{
		if( mem->Name->get_name() == name )
		{
			return mem.get();
		}
	}

	return nullptr;
}

void
Scope::add_named_value(String const& name, IdentifierValue id)
{
	names.insert(std::make_pair(name, id));
}

IdentifierValue*
Scope::get_identifier(std::string const& name)
{
	auto find_iter = names.find(name);
	if( find_iter == names.end() )
	{
		if( parent != nullptr )
		{
			return parent->get_identifier(name);
		}
		else
		{
			return nullptr;
		}
	}
	else
	{
		return &find_iter->second;
	}
}

llvm::Function*
Scope::get_function()
{
	auto Fn = Function;
	if( Fn == nullptr && parent != nullptr )
	{
		Fn = parent->get_function();
	}

	if( !Fn )
	{
		std::cout << "How?" << std::endl;
	}

	return Fn;
}

llvm::Value*
Codegen::promote_to_value(llvm::Value* Val)
{
	if( Val->getType()->isPointerTy() )
	{
		auto PointedToType = Val->getType()->getPointerElementType();
		return Builder->CreateLoad(PointedToType, Val);
	}
	else
	{
		// TODO: Should this be a logic error?
		return Val;
	}
}

/**
 * @brief 'type' always refers to llvm::llvm::Type, ast::TypeIdentifier is refered to as
 * TypeIdentifier
 *
 * @param type
 * @return llvm::llvm::Type*
 */
llvm::Type*
Codegen::get_type(ast::TypeDeclarator& type)
{
	// Need to iterate over type in reverse
	Vec<ast::TypeDeclarator const*> stack;

	for( auto const* ty = &type; ty != nullptr; ty = ty->get_base() )
	{
		stack.push_back(ty);
	}

	auto iter = stack.rbegin();
	auto base_type = *iter;
	iter++;

	auto BaseType = get_builtin_type(base_type->get_type_name());
	if( !BaseType )
	{
		auto type_identifier = current_scope->get_identifier(base_type->get_type_name());
		if( !type_identifier ||
			type_identifier->type != IdentifierValue::IdentifierType::struct_type )
		{
			std::cout << "Expected type name but got value name" << std::endl;
			return nullptr;
		}

		BaseType = type_identifier->data.type.TypeTy;
	}

	llvm::Type* RetType = BaseType;
	for( ; iter != stack.rend(); iter++ )
	{
		RetType = RetType->getPointerTo();
	}

	return RetType;
}

Codegen::Codegen()
{
	Context = std::make_unique<llvm::LLVMContext>();
	Module = std::make_unique<llvm::Module>("this_module", *Context);
	// Create a new builder for the module.
	Builder = std::make_unique<llvm::IRBuilder<>>(*Context);

	// Module global scope.
	current_scope = new Scope();
}

void
Codegen::visit(ast::Module const* node)
{
	for( auto const& s : node->statements )
	{
		s->visit(this);
	}
}

void
Codegen::visit(ast::Function const* node)
{
	// Transfer ownership of the prototype to the FunctionProtos map, but keep a
	// reference to it for use below.
	node->Proto->visit(this);

	if( node->Body.is_null() )
		return;

	auto TheFunctionIter = Functions.find(node->Proto->Name->get_name());
	if( TheFunctionIter == Functions.end() )
		return;

	// Create a new basic block to start insertion into.
	auto TheFunction = TheFunctionIter->second;
	BasicBlock* BB = BasicBlock::Create(*Context, "entry", TheFunction);
	Builder->SetInsertPoint(BB);

	new_scope(TheFunction);

	unsigned int idx = 0;
	for( auto& Arg : TheFunction->args() )
	{
		// Create an alloca for this variable.
		AllocaInst* Alloca = Builder->CreateAlloca(Arg.getType(), nullptr, Arg.getName());

		// Store the initial value into the alloca.
		Builder->CreateStore(&Arg, Alloca);

		// Add arguments to variable symbol table.
		auto id = node->Proto->Parameters->Parameters[idx]->Name.get();
		// auto ty = node->Proto->Parameters[idx]->llvm::Type.get();
		current_scope->add_named_value(id->get_name(), IdentifierValue{id, Alloca});
		idx++;
	}

	node->Body->visit(this);

	pop_scope();
}

void
Codegen::visit(ast::Block const* node)
{
	new_scope();
	for( auto& stmt : node->statements )
	{
		stmt->visit(this);
	}
	pop_scope();
}

void
Codegen::visit(ast::BinaryOperation const* node)
{
	node->LHS->visit(this);
	auto L = last_expr;
	last_expr = nullptr;

	node->RHS->visit(this);
	auto R = last_expr;
	last_expr = nullptr;

	if( !L || !R )
		return;

	L = promote_to_value(L);
	R = promote_to_value(R);

	if( !L->getType()->isIntegerTy() )
	{
		std::cout << "TODO: Typechecking, expecting int type" << std::endl;
		return;
	}

	if( !R->getType()->isIntegerTy() )
	{
		std::cout << "TODO: Typechecking, expecting int type" << std::endl;
		return;
	}

	// TODO: Height

	auto Op = node->Op;
	switch( Op )
	{
	case BinOp::plus:
		last_expr = Builder->CreateAdd(L, R, "addtmp");
		break;
	case BinOp::minus:
		last_expr = Builder->CreateSub(L, R, "subtmp");
		break;
	case BinOp::star:
		last_expr = Builder->CreateMul(L, R, "multmp");
		break;
	case BinOp::slash:
		last_expr = Builder->CreateSDiv(L, R, "divtmp");
		break;
	case BinOp::gt:
		last_expr = Builder->CreateICmpUGT(L, R, "cmptmp");
		break;
	case BinOp::gte:
		last_expr = Builder->CreateICmpUGE(L, R, "cmptmp");
		break;
	case BinOp::lt:
		last_expr = Builder->CreateICmpULT(L, R, "cmptmp");
		break;
	case BinOp::lte:
		last_expr = Builder->CreateICmpULE(L, R, "cmptmp");
		break;
	case BinOp::cmp:
		last_expr = Builder->CreateICmpEQ(L, R, "cmptmp");
		break;
	case BinOp::ne:
		last_expr = Builder->CreateICmpNE(L, R, "cmptmp");
		break;
	case BinOp::and_lex:
		last_expr = Builder->CreateAnd(L, R, "cmptmp");
		break;
	case BinOp::or_lex:
		last_expr = Builder->CreateOr(L, R, "cmptmp");
		break;
	default:
		return;
	}
}

void
Codegen::visit(ast::Number const* node)
{
	last_expr = ConstantInt::get(*Context, APInt(32, node->Val, true));
}

// Constant*
// geti8StrVal(M char const* str, Twine const& name)
// {
// 	LLVMContext& ctx = getGlobalContext();
// 	Constant* strConstant = ConstantDataArray::getString(ctx, str);
// 	GlobalVariable* GVStr = new GlobalVariable(
// 		M, strConstant->getType(), true, GlobalValue::InternalLinkage, strConstant, name);
// 	Constant* zero = Constant::getNullValue(IntegerType::getInt32Ty(ctx));
// 	Constant* indices[] = {zero, zero};
// 	Constant* strVal = ConstantExpr::getGetElementPtr(GVStr, indices, true);
// 	return strVal;
// }

void
Codegen::visit(ast::StringLiteral const* node)
{
	// auto array_type = ArrayType(ConstantInt::get(*Context, APInt(0, 8, true)), node->Val.size());

	// LLVMValueRef str = LLVMAddGlobal(module->getLLVMModule(), array_type, "");
	// ConstantData::set

	// LLVMSetInitializer(str, LLVMConstString(sourceString, size, true));
	// LLVMSetGlobalConstant(str, true);
	// LLVMSetLinkage(str, LLVMPrivateLinkage);
	// LLVMSetUnnamedAddress(str, LLVMGlobalUnnamedAddr);
	// LLVMSetAlignment(str, 1);

	// LLVMValueRef zeroIndex = LLVMConstInt(LLVMInt64Type(), 0, true);
	// LLVMValueRef indexes[2] = {zeroIndex, zeroIndex};

	// LLVMValueRef gep = LLVMBuildInBoundsGEP2(builder, strType, str, indexes, 2, "");
	auto Literal = ConstantDataArray::getString(*Context, node->Val.c_str(), true);

	GlobalVariable* GVStr = new GlobalVariable(
		*Module, Literal->getType(), true, GlobalValue::InternalLinkage, Literal);
	Constant* zero = Constant::getNullValue(IntegerType::getInt32Ty(*Context));
	Constant* indices[] = {zero, zero};
	Constant* strVal = ConstantExpr::getGetElementPtr(Literal->getType(), GVStr, indices);

	// auto MemberPtr = Builder->CreateGEP(
	// 	strVal->getType(), strVal, ConstantInt::get(*Context, APInt(32, 0, true)), "lit");
	last_expr = strVal;

	// return gep;
	// last_expr = ConstantArray::get(*Context, APInt(32, node->Val, true));
}

void
Codegen::visit(ast::Return const* node)
{
	auto Fn = current_scope->get_function();

	node->ReturnExpr->visit(this);

	if( Fn->getReturnType()->isVoidTy() )
	{
		if( last_expr )
		{
			std::cout << "Expected void return type" << std::endl;
		}
		else
		{
			Builder->CreateRetVoid();
		}
	}
	else
	{
		if( !last_expr )
		{
			std::cout << "Wrong return type!" << std::endl;
		}
		else
		{
			Builder->CreateRet(promote_to_value(last_expr));
			last_expr = nullptr;
		}
	}
}

llvm::Type*
Codegen::get_builtin_type(std::string const& name)
{
	if( name == "i8" || name == "u8" )
	{
		return llvm::Type::getInt8Ty(*Context);
	}
	else if( name == "i16" || name == "u16" )
	{
		return llvm::Type::getInt16Ty(*Context);
	}
	else if( name == "i32" || name == "u32" )
	{
		return llvm::Type::getInt32Ty(*Context);
	}
	else if( name == "void" )
	{
		return llvm::Type::getVoidTy(*Context);
	}
	else
	{
		return nullptr;
	}
}

void
Codegen::visit(ast::Prototype const* node)
{
	auto Name = node->Name->get_name();
	auto& Args = node->Parameters;

	std::vector<llvm::Type*> IRArguments;
	for( auto& arg : Args.get()->Parameters )
	{
		/**
		 * We do a special hack here. Since llvm can't pass structures by value,
		 * we just pass an array of
		 *
		 */
		auto ty = get_type(*arg->Type.get());
		if( ty == nullptr )
		{
			std::cout << "Unknown type" << std::endl;
			// TODO: Error;
			return;
		}

		IRArguments.push_back(ty);
	}

	auto ret_ty = get_type(*node->ReturnType.get());
	if( ret_ty == nullptr )
	{
		std::cout << "Unknown type" << std::endl;
		// TODO: Error;
		return;
	}

	FunctionType* FT = FunctionType::get(ret_ty, IRArguments, false);

	llvm::Function* F =
		llvm::Function::Create(FT, llvm::Function::ExternalLinkage, Name, Module.get());

	// Set names for all arguments.
	unsigned Idx = 0;
	for( auto& Arg : F->args() )
		Arg.setName(node->Parameters->Parameters[Idx++]->Name->get_name());

	Functions.insert(std::make_pair(Name, F));

	current_scope->add_named_value(Name, IdentifierValue{node->Name.get(), F});
}

void
Codegen::visit(ast::TypeIdentifier const* node)
{
	auto Var = current_scope->get_identifier(node->get_name());
	if( !Var )
	{
		std::cout << "Unrecognized type identifier: " << node->get_name() << std::endl;
	}
	return;
}

// Could be a.b.c
void
Codegen::visit(ast::ValueIdentifier const* node)
{
	// if( !current_scope->get_expected_type() )
	// {
	// 	std::cout << "Evaluating expression no-op" << std::endl;
	// 	return;
	// }

	auto value_identifier = current_scope->get_identifier(node->get_name());

	if( !value_identifier ||
		(value_identifier->type != IdentifierValue::IdentifierType::value &&
		 value_identifier->type != IdentifierValue::IdentifierType::function_type) )
	{
		std::cout << "Unexpected type name " << node->get_name() << std::endl;
		return;
	}

	// If the value is being used as an LValue, we want the Value itself.
	// TODO: Change the paradigm of last_expr to return the pointer to the value,
	// then if the value is needed, do a signle pointer deref.
	switch( value_identifier->type )
	{
	case IdentifierValue::IdentifierType::value:
		last_expr = value_identifier->data.Value;
		break;
	case IdentifierValue::IdentifierType::function_type:
		last_expr = value_identifier->data.Function;
		break;

	default:
		break;
	}
	// last_expr =
	// 	Builder->CreateLoad(Var->getAllocatedType(), Var, value_identifier->identifier->get_fqn());
}

void
Codegen::visit(ast::Let const* node)
{
	node->RHS->visit(this);
	// Get the last expression value somehow?
	if( last_expr == nullptr )
	{
		std::cout << "Expected expression" << std::endl;
		return;
	}

	auto R = last_expr;
	last_expr = nullptr;
	R = promote_to_value(R);

	// Create an alloca for this variable.

	llvm::Type* Type = nullptr;

	if( !node->Type->is_empty() )
	{
		Type = get_type(*node->Type.get());
	}
	else
	{
		Type = R->getType();
	}

	if( Type == nullptr )
	{
		std::cout << "Unknown type " << node->Type->get_name() << std::endl;
		return;
	}

	auto variable_name = node->Name->get_name();
	AllocaInst* Alloca = Builder->CreateAlloca(Type, nullptr, variable_name);

	// Add the value to the named scope AFTER the expression is genned.
	current_scope->add_named_value(variable_name, IdentifierValue{node->Name.get(), Alloca});

	Builder->CreateStore(R, Alloca);
}

// https://lists.llvm.org/pipermail/llvm-dev/2013-February/058880.html
void
Codegen::visit(ast::Struct const* node)
{
	Vec<llvm::Type*> members;
	for( auto& mem : node->MemberVariables )
	{
		auto ty = get_type(*mem->Type.get());
		if( !ty )
		{
			std::cout << "Type name unknown" << std::endl;
			return;
		}

		members.push_back(ty);
	}

	llvm::StructType* StructTy =
		llvm::StructType::create(*Context, members, node->TypeName->get_name());

	current_scope->add_named_value(
		node->TypeName->get_name(), IdentifierValue{node->TypeName.get(), StructTy, node});
}

void
Codegen::visit(ast::MemberReference const* node)
{
	node->base->visit(this);

	if( !last_expr )
	{
		std::cout << "Expected expression" << std::endl;
		return;
	}

	auto BaseValue = last_expr;
	last_expr = nullptr;

	// auto MemberType = get_type(node->);
	// if( !MemberType )
	// {
	// 	return;
	// }

	auto BaseType = BaseValue->getType();

	// We receive structs by reference, (i.e. struct*), so a pointer to a
	// struct is (struct**). Primitive value references are just themselves (i.e. int instead of
	// int*). So we always want to do ONE dereference.
	if( !BaseType->isPointerTy() )
	{
		std::cout << "Attempted to dereference non-reference" << std::endl;
		return;
	}

	// One free deref.
	// TODO: This should really be '->' (i.e. a separate node type)
	auto FreeDeref = BaseType->getPointerElementType();
	if( FreeDeref->isPointerTy() && FreeDeref->getPointerElementType()->isStructTy() )
	{
		BaseType = FreeDeref;
		BaseValue = Builder->CreateLoad(BaseType, BaseValue, "->");
	}

	// Now we check if this is a struct reference.
	if( !BaseType->isPointerTy() || !BaseType->getPointerElementType()->isStructTy() )
	{
		std::cout << "Attempted to dereference non-struct" << std::endl;
		return;
	}

	// Look up the struct definition; only need base type name to
	// look up the type definition and compute the GEP
	auto PointedToType = BaseType->getPointerElementType();
	auto StructName = PointedToType->getStructName();
	auto struct_name_str = String{StructName.data()};
	auto as_struct_id_val = current_scope->get_identifier(struct_name_str);
	if( as_struct_id_val->type != IdentifierValue::IdentifierType::struct_type )
	{
		std::cout << "???" << std::endl;
		return;
	}

	auto member_name = node->name->get_name();
	auto ast_struct_definition = as_struct_id_val->data.type.type_struct;
	auto idx = get_element_index_in_struct(ast_struct_definition, member_name);
	if( idx == -1 )
	{
		std::cout << "??!!" << std::endl;
		return;
	}

	auto MemberPtr = Builder->CreateStructGEP(
		PointedToType, BaseValue, idx, struct_name_str + "." + member_name);

	last_expr = MemberPtr;
}

void
Codegen::visit(ast::TypeDeclarator const* node)
{
	// No Op?
}

void
Codegen::visit(ast::If const* node)
{
	node->condition->visit(this);
	auto CondV = last_expr;
	last_expr = nullptr;
	if( !CondV )
		return;

	auto Function = current_scope->get_function();

	// Create blocks for the then and else cases.  Insert the 'then' block at the
	// end of the function.
	BasicBlock* ThenBB = BasicBlock::Create(*Context, "then", Function);
	BasicBlock* ElseBB = BasicBlock::Create(*Context, "else");
	BasicBlock* MergeBB = BasicBlock::Create(*Context, "ifcont");

	Builder->CreateCondBr(CondV, ThenBB, ElseBB);

	// Emit then value.
	Builder->SetInsertPoint(ThenBB);

	node->then_block->visit(this);
	auto ThenV = last_expr;
	last_expr = nullptr;

	Builder->CreateBr(MergeBB);
	Function->getBasicBlockList().push_back(ElseBB);
	Builder->SetInsertPoint(ElseBB);

	if( !node->else_block.is_null() )
	{
		node->else_block->visit(this);
		auto ElseV = last_expr;
		last_expr = nullptr;
	}
	Builder->CreateBr(MergeBB);

	// Codegen of 'Else' can change the current block, update ElseBB for the PHI.
	ElseBB = Builder->GetInsertBlock();

	// Emit merge block.
	Function->getBasicBlockList().push_back(MergeBB);
	Builder->SetInsertPoint(MergeBB);
}

void
Codegen::visit(ast::For const* node)
{
	node->init->visit(this);

	auto Function = current_scope->get_function();

	BasicBlock* LoopStartBB = BasicBlock::Create(*Context, "condition", Function);
	BasicBlock* LoopBB = BasicBlock::Create(*Context, "loop");
	BasicBlock* LoopDoneBB = BasicBlock::Create(*Context, "after_loop");

	// There are no implicit fallthroughs, we have to make an
	// explicit fallthrough from the current block to the LoopStartBB
	Builder->CreateBr(LoopStartBB);

	Builder->SetInsertPoint(LoopStartBB);
	node->condition->visit(this);
	auto CondV = last_expr;
	last_expr = nullptr;
	if( !CondV )
		return;

	Builder->CreateCondBr(CondV, LoopBB, LoopDoneBB);

	Function->getBasicBlockList().push_back(LoopBB);
	Builder->SetInsertPoint(LoopBB);

	node->body->visit(this);

	node->end_loop->visit(this);

	Builder->CreateBr(LoopStartBB);

	Function->getBasicBlockList().push_back(LoopDoneBB);
	Builder->SetInsertPoint(LoopDoneBB);
}

void
Codegen::visit(ast::Assign const* node)
{
	node->lhs->visit(this);

	auto LValue = last_expr;
	last_expr = nullptr;
	if( !LValue )
	{
		std::cout << "LValue undefined" << std::endl;
		return;
	}

	if( !LValue->getType()->isPointerTy() )
	{
		std::cout << "LValue must be an LValue! (a pointer type)" << std::endl;
		return;
	}

	// Store the initial value into the alloca.
	node->rhs->visit(this);
	auto RValue = last_expr;
	last_expr = nullptr;
	if( !RValue )
	{
		std::cout << "RLValue undefined" << std::endl;
		return;
	}
	RValue = promote_to_value(RValue);

	switch( node->Op )
	{
	case AssignOp::add:
	{
		auto LValuePromoted = promote_to_value(LValue);
		auto TempRValue = Builder->CreateAdd(RValue, LValuePromoted);
		Builder->CreateStore(TempRValue, LValue);
	}
	break;
	case AssignOp::sub:
	{
		auto LValuePromoted = promote_to_value(LValue);
		auto TempRValue = Builder->CreateSub(LValuePromoted, RValue);
		Builder->CreateStore(TempRValue, LValue);
	}
	break;
	case AssignOp::mul:
	{
		auto LValuePromoted = promote_to_value(LValue);
		auto TempRValue = Builder->CreateMul(RValue, LValuePromoted);
		Builder->CreateStore(TempRValue, LValue);
	}
	break;
	case AssignOp::div:
	{
		auto LValuePromoted = promote_to_value(LValue);
		auto TempRValue = Builder->CreateSDiv(RValue, LValuePromoted);
		Builder->CreateStore(TempRValue, LValue);
	}
	break;
	case AssignOp::assign:
		Builder->CreateStore(RValue, LValue);
		break;
	}
}

void
Codegen::visit(ast::While const* node)
{
	auto Function = current_scope->get_function();

	BasicBlock* LoopConditionCheckBB = BasicBlock::Create(*Context, "loop_condition", Function);
	BasicBlock* LoopBB = BasicBlock::Create(*Context, "loop");
	BasicBlock* AfterLoopBB = BasicBlock::Create(*Context, "after_loop");

	// Insert an explicit fall through from the current block to the LoopBB.
	// TODO: (2022-01-11) I'm not sure why this is needed, but LLVM segfaults if its not there.
	// Maybe it is doing some sort of graph analysis and implicit fallthrough on blocks
	// is not allowed?
	// Answer: (2022-05-25) Yes. It is. Every block must end with a terminator instruction, the
	// block we are in when we enter this function (i.e. visit(ast:While)) does not yet have a
	// terminator instr.
	Builder->CreateBr(LoopConditionCheckBB);

	Builder->SetInsertPoint(LoopConditionCheckBB);
	node->condition->visit(this);
	auto CondV = last_expr;
	last_expr = nullptr;
	if( !CondV )
		return;

	Builder->CreateCondBr(CondV, LoopBB, AfterLoopBB);

	Function->getBasicBlockList().push_back(LoopBB);
	Builder->SetInsertPoint(LoopBB);
	node->loop_block->visit(this);
	Builder->CreateBr(LoopConditionCheckBB);

	Function->getBasicBlockList().push_back(AfterLoopBB);
	Builder->SetInsertPoint(AfterLoopBB);
}

void
Codegen::visit(ast::Call const* node)
{
	node->call_target->visit(this);
	if( !last_expr )
	{
		return;
	}

	if( !last_expr->getType()->isPointerTy() &&
		!last_expr->getType()->getPointerElementType()->isFunctionTy() )
	{
		std::cout << "Expected function type" << std::endl;
		return;
	}

	llvm::Function* Function = static_cast<llvm::Function*>(last_expr);

	std::vector<Value*> ArgsV;
	for( unsigned i = 0, e = node->args.args.size(); i != e; ++i )
	{
		node->args.args[i]->visit(this);
		auto Expr = last_expr;
		last_expr = nullptr;
		if( !Expr )
		{
			std::cout << "Expected expr in arg" << std::endl;
			return;
		}

		// Expr = promote_to_value(Expr);
		ArgsV.push_back(Expr);
	}

	// https://github.com/ark-lang/ark/issues/362
	if( Function->getReturnType()->isVoidTy() )
	{
		auto tp1 = Function->getFunctionType()->getParamType(0);
		auto tp2 = ArgsV[0]->getType();
		if( tp1 == tp2 )
		{
			std::cout << " ???" << std::endl;
		}
		last_expr = Builder->CreateCall(Function, ArgsV);
	}
	else
	{
		last_expr = Builder->CreateCall(Function, ArgsV, "call");
	}
}

void
Codegen::visit(ast::Statement const* node)
{
	node->stmt->visit(this);
}

void
Codegen::visit(ast::Expression const* node)
{
	node->base->visit(this);
}

void
Codegen::pop_scope()
{
	auto parent_scope = current_scope->get_parent();
	if( !parent_scope )
	{
		// TODO: Error
	}

	delete current_scope;
	current_scope = parent_scope;
}

void
Codegen::new_scope(llvm::Function* Fn)
{
	Scope* scope = new Scope(current_scope, Fn);

	current_scope = scope;
}