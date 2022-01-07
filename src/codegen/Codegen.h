#pragma once

#include "../ast/IAstVisitor.h"
#include "../ast/ast.h"
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>

#include <map>
#include <string>
#include <vector>

// TODO: HIR with types

namespace codegen
{
class TypedIdentifier
{
public:
	ast::Identifier const* name;
	ast::Identifier const* type;

	// TODO: Separate this out...
	llvm::AllocaInst* Value;

	TypedIdentifier(
		ast::Identifier const* name, ast::Identifier const* type, llvm::AllocaInst* value)
		: name(name)
		, type(type)
		, Value(value){};
};

class Scope
{
	llvm::Function* Function = nullptr;
	Scope* parent = nullptr;

	std::map<std::string, TypedIdentifier> names;

public:
	Scope(){};
	Scope(Scope* parent)
		: parent(parent){};
	Scope(Scope* parent, llvm::Function* Function)
		: parent(parent)
		, Function(Function){};

	void add_named_value(TypedIdentifier id);
	TypedIdentifier* get_named_value(std::string const& name);

	Scope* get_parent() { return parent; }

	llvm::Function* get_function();
};

class Codegen : public IAstVisitor
{
	llvm::Value* last_expr = nullptr;
	Scope* current_scope;
	std::map<std::string, llvm::Function*> Functions;
	std::unique_ptr<llvm::LLVMContext> Context;
	std::unique_ptr<llvm::IRBuilder<>> Builder;

public:
	std::unique_ptr<llvm::Module> Module;
	Codegen();
	virtual void visit(ast::Module const*) override;
	virtual void visit(ast::Function const*) override;
	virtual void visit(ast::Block const*) override;
	virtual void visit(ast::BinaryOperation const*) override;
	virtual void visit(ast::Number const*) override;
	virtual void visit(ast::Return const*) override;
	virtual void visit(ast::Prototype const*) override;
	virtual void visit(ast::TypeIdentifier const*) override;
	virtual void visit(ast::ValueIdentifier const*) override;
	virtual void visit(ast::Let const*) override;
	virtual void visit(ast::Struct const*) override;

private:
	void pop_scope();
	void new_scope(llvm::Function* Fn = nullptr);

	llvm::Type* get_builtin_type(std::string const& name);
};
} // namespace codegen
