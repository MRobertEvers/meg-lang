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
	ast::Identifier* identifier = nullptr;

	enum class IdentifierType
	{
		struct_type,
		function_type,
		value
	} type = IdentifierType::value;
	union Data
	{
		llvm::AllocaInst* Value;

		llvm::Function* Function;

		// Accompanys TypeTy;
		struct TypeIdentifierValue
		{
			llvm::Type* TypeTy;
			ast::Struct const* type_struct;

			TypeIdentifierValue(llvm::Type* TypeTy, ast::Struct const* type_struct)
				: TypeTy(TypeTy)
				, type_struct(type_struct){};
		} type;

		Data(llvm::Function* Function)
			: Function(Function)
		{}
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
		, type(IdentifierType::struct_type)
		, data(TypeTy, type_struct){};

	IdentifierValue(ast::Identifier* identifier, llvm::Function* Function)
		: identifier(identifier)
		, type(IdentifierType::function_type)
		, data(Function){};
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
	virtual void visit(ast::If const*) override;
	virtual void visit(ast::Assign const*) override;
	virtual void visit(ast::While const*) override;
	virtual void visit(ast::Call const*) override;

private:
	void pop_scope();
	void new_scope(llvm::Function* Fn = nullptr);

	llvm::Type* get_builtin_type(std::string const& name);

	llvm::Type* get_type(ast::Type const& name);

	llvm::Value* promote_to_value(llvm::Value* val);
};
} // namespace codegen
