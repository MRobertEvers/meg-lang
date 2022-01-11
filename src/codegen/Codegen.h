#pragma once
#include "ast/ast.h"
#include "common/String.h"
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>

#include <map>
namespace codegen
{

class IdentifierValue
{
public:
	ast::Identifier* identifier;

	bool is_type_name = false;
	union Data
	{
		llvm::AllocaInst* Value;

		// Accompanys TypeTy;
		struct TypeIdentifierValue
		{
			llvm::Type* TypeTy;
			ast::Struct const* type_struct;

			TypeIdentifierValue(llvm::Type* TypeTy, ast::Struct const* type_struct)
				: TypeTy(TypeTy)
				, type_struct(type_struct){};
		} type;

		Data(llvm::AllocaInst* Value)
			: Value(Value)
		{}
		Data(llvm::Type* TypeTy, ast::Struct const* type_struct)
			: type(TypeTy, type_struct)
		{}
	} data;

	IdentifierValue(ast::Identifier* identifier, llvm::AllocaInst* Value)
		: identifier(identifier)
		, data(Value){};

	IdentifierValue(ast::Identifier* identifier, llvm::Type* TypeTy, ast::Struct const* type_struct)
		: identifier(identifier)
		, is_type_name(true)
		, data(TypeTy, type_struct){};
};

class Scope
{
	llvm::Function* Function = nullptr;
	Scope* parent = nullptr;
	llvm::Type* expected_type = nullptr;

	std::map<String, IdentifierValue> names;

public:
	Scope(){};
	Scope(Scope* parent)
		: parent(parent){};
	Scope(Scope* parent, llvm::Function* Function)
		: parent(parent)
		, Function(Function){};

	void add_named_value(String const& name, IdentifierValue id);
	IdentifierValue* get_identifier(std::string const& name);

	Scope* get_parent() { return parent; }

	llvm::Function* get_function();

	llvm::Type* get_expected_type() { return expected_type; };
	void set_expected_type(llvm::Type* type) { expected_type = type; }
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
	virtual void visit(ast::MemberReference const*) override;
	virtual void visit(ast::TypeDeclarator const*) override;

private:
	void pop_scope();
	void new_scope(llvm::Function* Fn = nullptr);

	llvm::Type* get_builtin_type(std::string const& name);

	llvm::Type* get_type(ast::Type const& name);
};
} // namespace codegen
