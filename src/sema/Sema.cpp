#include "Sema.h"

#include "ast/ast.h"

using namespace sema;

Sema::ScopedType
Sema::create_type(Type ty)
{
	if( ty.is_pointer_type() )
	{
		auto find = types.find(ty.get_name());
		if( find != types.end() )
		{
			return ScopedType{find->second, nullptr};
		}
		else
		{
			return scopes[0].add_named_value(ty.get_name(), new Type{ty});
		}
	}
	else
	{
		auto iter = types.emplace(ty.get_name(), new Type{ty});

		return add_named_value(ty.get_name(), iter.first->second);
	}
}

Sema::Scope::Scope(){

};

Sema::Scope::Scope(Scope* parent)
	: parent(parent){

	  };

Sema::Scope::~Scope()
{
	//
}

Sema::ScopedType
Sema::Scope::add_named_value(String const& name, Type* id)
{
	auto iter = names.emplace(name, id);

	return ScopedType{iter.first->second, this};
}

Type const*
Sema::Scope::lookup(String const& name) const
{
	auto me = names.find(name);
	if( me == names.end() )
	{
		if( parent == nullptr )
		{
			return nullptr;
		}
		else
		{
			return parent->lookup(name);
		}
	}
	else
	{
		return me->second;
	}
}

Sema::Scope*
Sema::Scope::get_parent()
{
	return parent;
}

Sema::Sema()
	: last_expr(SemaError(""))
{
	scopes.reserve(1000);
	new_scope();

	create_type(void_type);
	create_type(infer_type);
	create_type(i8_type);
	create_type(i16_type);
	create_type(i32_type);
	create_type(u8_type);
	create_type(u16_type);
	create_type(u32_type);
}

Sema::~Sema()
{}

void
Sema::visit(ast::Module const* node)
{
	for( auto& stmt : node->statements )
	{
		visit_node(stmt.get());
	}
}

void
Sema::visit(ast::Function const* node)
{
	new_scope();
	visit_node(node->Proto.get());
	visit_node(node->Body.get());
	pop_scope();
}

void
Sema::visit(ast::Block const* node)
{
	new_scope();
	for( auto& stmt : node->statements )
	{
		visit_node(stmt.get());
	}
	pop_scope();
}

void
Sema::visit(ast::BinaryOperation const* node)
{
	visit_node(node->LHS.get());
	auto lhs_type = std::move(last_expr);
	last_expr = SemaError("");
	if( !lhs_type.ok() )
	{
		return;
	}

	visit_node(node->RHS.get());
	auto rhs_type = std::move(last_expr);
	last_expr = SemaError("");
	if( !rhs_type.ok() )
	{
		return;
	}

	auto lhs_expr_type = lhs_type.unwrap();
	auto rhs_expr_type = rhs_type.unwrap();
	if( lhs_expr_type->expr != rhs_expr_type->expr )
	{
		last_expr = SemaError("Mismatched types.");
	}
	else
	{
		last_expr = lhs_expr_type;
	}
}

void
Sema::visit(ast::Number const* node)
{
	//
}

void
Sema::visit(ast::Return const* node)
{
	//
}

void
Sema::visit(ast::Prototype const* node)
{
	Vec<Type const*> args;
	for( auto& arg : node->Parameters->Parameters )
	{
		auto& name_identifier = arg->Name;
		auto& type_declarator = arg->Type;

		auto base_type = lookup(type_declarator->get_type_name());
		if( !base_type )
		{
			last_expr = SemaError("Unknown type");
			return;
		}

		auto type = base_type;
		ast::TypeDeclarator const* p_type = type_declarator.get();
		while( p_type->is_pointer_type() )
		{
			auto scoped = create_type(Type::PointerTo(*type));

			type = scoped.expr;

			p_type = p_type->get_base();
		}

		args.push_back(type);
	}

	auto return_type_declarator = node->ReturnType.get();
	auto return_type = lookup(return_type_declarator->get_type_name());
	if( !return_type )
	{
		last_expr = SemaError("Unknown type");
		return;
	}

	auto function_type_def = Type::Function(node->Name->get_name(), args, return_type);
	create_type(function_type_def);
}

void
Sema::visit(ast::TypeIdentifier const* node)
{
	//
}

void
Sema::visit(ast::ValueIdentifier const* node)
{
	//
}

void
Sema::visit(ast::Let const* node)
{
	//
}

void
Sema::visit(ast::Struct const* node)
{
	std::map<String, Type const*> members;
	for( auto& member : node->MemberVariables )
	{
		auto& name_identifier = member->Name;
		auto& type_declarator = member->Type;

		auto base_type = lookup(type_declarator->get_type_name());
		if( !base_type )
		{
			last_expr = SemaError("Unknown type");
			return;
		}

		auto type = base_type;
		ast::TypeDeclarator const* p_type = type_declarator.get();
		while( p_type->is_pointer_type() )
		{
			auto scoped = create_type(Type::PointerTo(*type));

			type = scoped.expr;

			p_type = p_type->get_base();
		}

		members.emplace(name_identifier->get_name(), type);
	}

	// TODO: Check if struct already exists
	auto struct_type_def = Type::Struct(node->TypeName->get_name(), members);
	create_type(struct_type_def);
}

void
Sema::visit(ast::MemberReference const* node)
{
	//
}

void
Sema::visit(ast::TypeDeclarator const* node)
{
	//
}

void
Sema::visit(ast::If const* node)
{
	//
}

void
Sema::visit(ast::Assign const* node)
{
	//
}

void
Sema::visit(ast::While const* node)
{
	//
}

void
Sema::visit(ast::Call const* node)
{
	//
}

void
Sema::visit(ast::Statement const* node)
{
	//
}

void
Sema::visit(ast::Expression const* node)
{
	//
}

void
Sema::visit_node(ast::IAstNode const* node)
{
	node->visit(this);
}

Sema::ScopedType
Sema::add_named_value(String const& name, Type* id)
{
	return current_scope->add_named_value(name, id);
}

Type const*
Sema::lookup(String const& name)
{
	return current_scope->lookup(name);
}

void
Sema::new_scope()
{
	scopes.emplace_back(current_scope);
	current_scope = &scopes.back();
}

void
Sema::pop_scope()
{
	current_scope->is_in_scope = false;
	current_scope = current_scope->get_parent();
}
