#include "Sema.h"

#include "ast/ast.h"

using namespace sema;

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
Sema::Scope::add_named_value(String const& name, Type id)
{
	auto iter = names.emplace(name, id);

	return ScopedType{&iter.first->second, this};
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
		return &me->second;
	}
}

Sema::Scope*
Sema::Scope::get_parent()
{
	return parent;
}

Sema::Sema()
{
	new_scope();

	add_named_value(void_type.name, void_type);
	add_named_value(infer_type.name, infer_type);
	add_named_value(i8_type.name, i8_type);
	add_named_value(i16_type.name, i16_type);
	add_named_value(i32_type.name, i32_type);
	add_named_value(u8_type.name, u8_type);
	add_named_value(u16_type.name, u16_type);
	add_named_value(u32_type.name, u32_type);
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

	pop_scope();
}

void
Sema::visit(ast::BinaryOperation const* node)
{
	//
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
	for( auto& arg : node->Parameters->Parameters )
	{
		auto& name_identifier = arg->Name;
		auto& type_declarator = arg->Type;

		auto type = lookup(type_declarator->get_type_name());
		if( !type )
		{
			last_expr = SemaError("Unknown type");
			return;
		}
		add_named_value(name_identifier->get_name(), *type);
	}

	auto return_type_declarator = node->ReturnType;
	auto type = lookup(return_type_declarator->get_type_name());
	if( !type )
	{
		last_expr = SemaError("Unknown type");
		return;
	}
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
	//
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
Sema::add_named_value(String const& name, Type id)
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
