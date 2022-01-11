#include "Codegen.h"

using namespace llvm;
using namespace codegen;
using namespace ast;

static int
get_element_index_in_struct(ast::Struct const* st, std::string const& name)
{
	unsigned int idx = 0;
	for( auto& mem : st->MemberVariables )
	{
		if( mem->Name->get_fqn() == name )
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
		if( mem->Name->get_fqn() == name )
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

// TODO: Remove this.
llvm::Value*
Codegen::load_if_gep(llvm::Value* val, ast::IExpressionNode const* node)
{
	return val;
	// if( !val->hasValueHandle() )
	// {
	// 	auto RHSType = get_type(node->get_type());
	// 	if( !RHSType )
	// 	{
	// 		std::cout << "No Type!!!" << std::endl;
	// 		return val;
	// 	}
	// 	return Builder->CreateLoad(RHSType, val);
	// }
	// else
	// {
	// 	return val;
	// }
}

/**
 * @brief 'type' always refers to llvm::llvm::Type, ast::TypeIdentifier is refered to as
 * TypeIdentifier
 *
 * @param type
 * @return llvm::llvm::Type*
 */
llvm::Type*
Codegen::get_type(ast::Type const& type)
{
	// Need to iterate over type in reverse
	Vec<ast::Type const*> stack;

	for( auto ty = &type; ty != nullptr; ty = ty->base )
	{
		stack.push_back(ty);
	}

	auto iter = stack.rbegin();
	auto base_type = *iter;
	iter++;

	auto BaseType = get_builtin_type(base_type->name);
	if( !BaseType )
	{
		auto type_identifier = current_scope->get_identifier(base_type->name);
		if( !type_identifier || !type_identifier->is_type_name )
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
		// auto ty = node->Proto->Parameters[idx]->llvm::Type.get();
		current_scope->add_named_value(id->get_fqn(), IdentifierValue{id, Alloca});
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
	L = load_if_gep(L, node->LHS.get());

	node->RHS->visit(this);
	auto R = last_expr;
	last_expr = nullptr;
	R = load_if_gep(R, node->RHS.get());

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
			Builder->CreateRet(load_if_gep(last_expr, node->ReturnExpr.get()));
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

	std::vector<llvm::Type*> IRArguments;
	for( auto& arg : Args )
	{
		/**
		 * We do a special hack here. Since llvm can't pass structures by value,
		 * we just pass an array of
		 *
		 */
		auto ty = get_type(arg->Type->get_type());
		if( ty == nullptr )
		{
			std::cout << "Unknown type" << std::endl;
			// TODO: Error;
			return;
		}

		IRArguments.push_back(ty);
	}

	auto ret_ty = get_type(node->ReturnType->get_type());
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
		Arg.setName(node->Parameters[Idx++]->Name->get_fqn());

	Functions.insert(std::make_pair(Name, F));
}

void
Codegen::visit(ast::TypeIdentifier const* node)
{
	auto Var = current_scope->get_identifier(node->get_fqn());
	if( !Var )
	{
		std::cout << "Unrecognized type identifier: " << node->get_fqn() << std::endl;
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

	auto value_identifier = current_scope->get_identifier(node->get_fqn());

	if( value_identifier->is_type_name )
	{
		std::cout << "Unexpected type name" << std::endl;
		return;
	}
	auto Var = value_identifier->data.Value;
	last_expr =
		Builder->CreateLoad(Var->getAllocatedType(), Var, value_identifier->identifier->get_fqn());
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
	R = load_if_gep(R, node->RHS.get());

	// Create an alloca for this variable.
	auto& type = node->Type->get_type();
	llvm::Type* Type = get_type(type);
	if( Type == nullptr )
	{
		std::cout << "Unknown type " << type.name << std::endl;
		return;
	}

	auto variable_name = node->Name->get_fqn();
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
		auto ty = get_type(mem->Type->get_type());
		if( !ty )
		{
			std::cout << "Type name unknown" << std::endl;
			return;
		}

		members.push_back(ty);
	}

	llvm::StructType* StructTy =
		llvm::StructType::create(*Context, members, node->TypeName->get_fqn());

	current_scope->add_named_value(
		node->TypeName->get_fqn(), IdentifierValue{node->TypeName.get(), StructTy, node});
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

	auto& ast_type = node->get_type();
	auto MemberType = get_type(ast_type);
	if( !MemberType )
	{
		return;
	}

	auto BaseType = BaseValue->getType();
	auto PointedToType = BaseType;

	// TODO: This allows a free dereference if needed.
	if( BaseType->isPointerTy() )
	{
		PointedToType = BaseType->getPointerElementType();
	}
	else
	{
		BaseValue->hasValueHandle();
		// BaseValue = Builder->CreateLoad(
		// 	Var->getAllocatedType(), Var, value_identifier->identifier->get_fqn());
	}

	if( !PointedToType->isStructTy() )
	{
		std::cout << "Attempted to dereference non-struct" << std::endl;
		return;
	}

	// Look up the struct definition
	auto StructName = PointedToType->getStructName();
	auto struct_name_str = String{StructName.data()};
	auto as_struct_id_val = current_scope->get_identifier(struct_name_str);
	if( !as_struct_id_val->is_type_name )
	{
		std::cout << "???" << std::endl;
		return;
	}
	auto member_name = node->name->get_fqn();
	auto ast_struct_definition = as_struct_id_val->data.type.type_struct;
	auto idx = get_element_index_in_struct(ast_struct_definition, member_name);
	// auto ast_member_decl = get_member_of_struct(ast_struct_definition, member_name);
	// auto MemberType = get_type(ast_member_decl->Type->get_type());
	if( idx == -1 )
	{
		std::cout << "??!!" << std::endl;
		return;
	}

	auto MemberPtr = Builder->CreateStructGEP(
		PointedToType, BaseValue, idx, struct_name_str + "." + member_name);

	// If the type of MemberPtr is a struct pointer, don't load it.
	// Most operations in llvm work on struct pointers, not the struct itself.
	// This makes sense too because if
	if( MemberPtr->getType()->getPointerElementType()->isStructTy() )
	{
		// Do nothing
		last_expr = MemberPtr;
	}
	else
	{
		// Load the value.
		last_expr = Builder->CreateLoad(MemberType, MemberPtr, "");
	}

	// auto member_ty = get_type(member->llvm::Type->get_fqn());
	// if( idx == -1 || !member || !member_ty )
	// {
	// 	std::cout << "Unrecognized member variable " << nm << std::endl;
	// 	return;
	// }

	// std::vector<llvm::llvm::Type*> members;
	// for( auto& mem : node->MemberVariables )
	// {
	// 	auto ty = get_type(mem->llvm::Type->get_fqn());
	// 	if( !ty )
	// 	{
	// 		std::cout << "llvm::Type name unknown" << std::endl;
	// 		return;
	// 	}

	// 	members.push_back(ty);
	// }

	// llvm::StructType* StructTy =
	// 	llvm::StructType::create(*Context, members, node->TypeName->get_fqn());

	// current_scope->add_named_value(
	// 	TypedIdentifier{node->TypeName.get(), node->TypeName.get(), StructTy, node});
}

void
Codegen::visit(ast::TypeDeclarator const* node)
{
	// No Op?
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