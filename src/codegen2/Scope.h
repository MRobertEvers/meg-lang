#pragma once
#include "LValue.h"
#include "common/String.h"

#include <map>
#include <optional>

namespace cg
{
class Scope
{
	std::map<String, LValue> values;
	Scope* parent = nullptr;

public:
	Scope(Scope* parent);

	void add_lvalue(String const& name, LValue lvalue);
	std::optional<LValue> lookup(String const& name) const;
	Scope* get_parent() const;
};
} // namespace cg