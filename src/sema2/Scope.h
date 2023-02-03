#pragma once
#include "Type.h"
#include "common/String.h"

#include <map>
namespace sema
{

class Scope
{
	Scope* parent = nullptr;

	std::map<String, Type const*> types;
	std::map<String, TypeInstance> names;
	TypeInstance expected_return;

public:
	bool is_in_scope = true;

	Scope();
	Scope(Scope* parent);
	~Scope();

	void add_value_identifier(String const& name, TypeInstance id);
	void add_type_identifier(Type const* id);
	Type const* lookup_type(String const& name) const;
	TypeInstance const* lookup_value_type(String const& name) const;
	TypeInstance const* get_expected_return() const;
	void set_expected_return(TypeInstance n);
	Scope* get_parent();
};
} // namespace sema