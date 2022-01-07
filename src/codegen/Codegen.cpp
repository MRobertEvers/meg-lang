#include "Codegen.h"

#include "../ast/ast.h"

#include <iostream>

using namespace llvm;

using namespace codegen;

void
Scope::add_named_value(TypedIdentifier id)
{
	// TODO: Check existing names
	names.insert(std::make_pair(id.name->get_fqn(), id));
}

TypedIdentifier*
Scope::get_named_value(std::string const& name)
{
	auto find_iter = names.find(name);
	if( find_iter == names.end() )
	{
		if( parent != nullptr )
		{
			return parent->get_named_value(name);
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

Codegen::Codegen()
{
	Context = std::make_unique<llvm::LLVMContext>();
	Module = std::make_unique<llvm::Module>("this_module", *Context);
	// Create a new builder for the module.
	Builder = std::make_unique<llvm::IRBuilder<>>(*Context);
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

	auto TheFunctionIter = Functions.find(node->Proto->Name->get_fqn());
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
		auto id = node->Proto->Parameters[idx]->Name.get();
		auto ty = node->Proto->Parameters[idx]->Type.get();
		current_scope->add_named_value(TypedIdentifier{id, ty, Alloca});
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
	case '+':
		last_expr = Builder->CreateAdd(L, R, "addtmp");
		break;
	case '-':
		last_expr = Builder->CreateSub(L, R, "subtmp");
		break;
	case '*':
		last_expr = Builder->CreateMul(L, R, "multmp");
		break;
	case '/':
		last_expr = Builder->CreateSDiv(L, R, "divtmp");
		break;
	case '<':
		last_expr = Builder->CreateICmpULT(L, R, "cmptmp");
		break;
	default:
		return;
	}
}

void
Codegen::visit(ast::Number const* node)
{
	// TODO: Current expected type
	last_expr = ConstantInt::get(*Context, APInt(32, node->Val, true));
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
			Builder->CreateRet(last_expr);
			last_expr = nullptr;
		}
	}
}

Type*
Codegen::get_builtin_type(std::string const& name)
{
	if( name == "i8" || name == "u8" )
	{
		return Type::getInt8Ty(*Context);
	}
	else if( name == "i16" || name == "u16" )
	{
		return Type::getInt16Ty(*Context);
	}
	else if( name == "i32" || name == "u32" )
	{
		return Type::getInt32Ty(*Context);
	}
	else
	{
		return nullptr;
	}
}

void
Codegen::visit(ast::Prototype const* node)
{
	auto Name = node->Name->get_fqn();
	auto& Args = node->Parameters;

	std::vector<Type*> IRArguments;
	for( auto& arg : Args )
	{
		auto ty = get_builtin_type(arg->Type->get_fqn());
		if( ty == nullptr )
		{
			std::cout << "Unknown type" << std::endl;
			// TODO: Error;
			return;
		}
		IRArguments.push_back(ty);
	}

	auto ret_ty = get_builtin_type(node->ReturnType->get_fqn());
	if( ret_ty == nullptr )
	{
		std::cout << "Unknown type" << std::endl;
		// TODO: Error;
		return;
	}

	FunctionType* FT = FunctionType::get(ret_ty, IRArguments, false);

	Function* F = Function::Create(FT, Function::ExternalLinkage, Name, Module.get());

	// Set names for all arguments.
	unsigned Idx = 0;
	for( auto& Arg : F->args() )
		Arg.setName(node->Parameters[Idx++]->Name->get_fqn());

	Functions.insert(std::make_pair(Name, F));
}

void
Codegen::visit(ast::TypeIdentifier const* node)
{
	auto Var = current_scope->get_named_value(node->get_fqn());
	if( !Var )
	{
		std::cout << "Unrecognized type identifier: " << node->get_fqn() << std::endl;
	}
	return;
}

void
Codegen::visit(ast::ValueIdentifier const* node)
{
	auto Var = current_scope->get_named_value(node->get_fqn());
	if( !Var )
	{
		std::cout << "Unrecognized value identifier: " << node->get_fqn() << std::endl;
	}
	last_expr =
		Builder->CreateLoad(Var->Value->getAllocatedType(), Var->Value, Var->name->get_fqn());
	return;
}

void
Codegen::visit(ast::Let const* node)
{
	// Create an alloca for this variable.
	auto builtin_ty = get_builtin_type(node->Type->get_fqn());
	if( builtin_ty == nullptr )
	{
		std::cout << "Error type node found: " << node->Type->get_fqn() << std::endl;
		return;
	}

	AllocaInst* Alloca = Builder->CreateAlloca(builtin_ty, nullptr, node->Name->get_fqn());

	node->RHS->visit(this);

	// Get the last expression value somehow?
	if( last_expr == nullptr )
	{
		std::cout << "Expected expression" << std::endl;
		return;
	}

	// Add the value to the named scope AFTER the expression is genned.
	current_scope->add_named_value(TypedIdentifier{node->Name.get(), node->Type.get(), Alloca});

	Builder->CreateStore(last_expr, Alloca);
	last_expr = nullptr;
}

void
Codegen::visit(ast::Struct const*)
{}

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