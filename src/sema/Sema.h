#pragma once
#include "SemaResult.h"
#include "Type.h"
#include "ast/IAstNode.h"
#include "ast/IAstVisitor.h"
#include "common/Vec.h"

#include <map>

namespace sema
{

/**
 * @brief Performs semantic analysis on the AST produced by parser and produces HIR
 *
 * Since we are using a visit pattern that returns void,
 * there are a few conceptual helpers to make seem like we
 * can call visit and get a return value.
 *
 * First, each node is either a statement or an expression.
 * Nodes can return statements through the 'last_stmt' and
 * expressions through 'last_expr'. Just like the parser
 * however, the last result is return via a SemaResult<T>
 * and should be checked before continuing.
 */
class Sema : public IAstVisitor
{
	// Global types. All types are unique and stored here.
	std::map<String, Type*> types;

	class Scope;
	struct ScopedType
	{
		Type* expr = nullptr;
		// Scope may be nullptr if the type is
		// a pointer type.
		Scope* scope = nullptr;
		ScopedType(){};
		ScopedType(Type* expr, Scope* scope)
			: expr(expr)
			, scope(scope){};

		void clear()
		{
			expr = nullptr;
			scope = nullptr;
		}
		bool is_null() { return expr == nullptr; }
	};

	class Scope
	{
		Scope* parent = nullptr;

		std::map<String, Type*> names;

	public:
		bool is_in_scope = true;

		Scope();
		Scope(Scope* parent);
		~Scope();

		ScopedType add_named_value(String const& name, Type* id);
		Type const* lookup(String const& name) const;

		Scope* get_parent();
	};

	// TODO: This is shaky
	Vec<Scope> scopes;
	Scope* current_scope = nullptr;

	SemaResult<ScopedType> last_expr;
	// SemaResult<ScopedType> last_err;

	Sema::ScopedType create_type(Type ty);

public:
	Sema();
	~Sema();
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
	virtual void visit(ast::Statement const*) override;
	virtual void visit(ast::Expression const*) override;

private:
	void visit_node(ast::IAstNode const* node);
	void add_type(Type type);
	ScopedType add_named_value(String const& name, Type* id);
	Type const* lookup(String const& name);
	void new_scope();
	void pop_scope();
};
} // namespace sema