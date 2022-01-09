#pragma once

namespace codegen
{
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

	llvm::Type* get_type(std::string const& name);
};
} // namespace codegen
