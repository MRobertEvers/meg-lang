#pragma once
#include "Types.h"
#include "common/String.h"
#include "type/Type.h"

#include <map>
#include <optional>
namespace sema
{
// TODO: ScopeExitType => return, yield, etc.
class Scope
{
	Scope* parent = nullptr;
	std::map<String, Type const*> types_in_scope;

	Types* types;
	std::map<String, TypeInstance> names;
	std::optional<TypeInstance> expected_return;

public:
	bool is_in_scope = true;

	Scope(Types* parent);
	Scope(Scope* parent);

	void add_value_identifier(String const& name, TypeInstance id);
	void add_type_identifier(Type const* id);
	Type const* lookup_type(String const& name) const;
	TypeInstance const* lookup_value_type(String const& name) const;
	std::optional<TypeInstance> get_expected_return() const;
	void set_expected_return(TypeInstance n);
	void clear_expected_return();
	Scope* get_parent();
};
} // namespace sema