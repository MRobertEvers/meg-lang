#pragma once
#include "QualifiedName.h"
#include "ir/Name.h"

#include <string>
#include <vector>

namespace sema
{

class NameLookupResult
{
	enum LookupResultKind
	{
		RNotFound = 0,
		RName = 1
	} kind;

	union
	{
		ir::NameRef name_;
	};

public:
	NameLookupResult()
		: kind(RNotFound)
	{}

	NameLookupResult(ir::NameRef name)
		: name_(name)
		, kind(RName)
	{}

	bool is_found() const { return kind != RNotFound; }

	ir::NameRef result() const
	{
		assert(is_found());
		return name_;
	}
};

class SemaLookup
{
	std::vector<ir::Name> names;

	ir::NameRef root_namespace;

	std::vector<ir::NameRef> namespace_stack;

public:
	SemaLookup();

	NameLookupResult lookup(QualifiedName const& name);
	NameLookupResult lookup_decl(QualifiedName const& name);
	NameLookupResult lookup(QualifiedName const& name, ir::NameRef nspace);

	ir::NameRef add_name(ir::Name name);
	ir::NameRef add_name(ir::NameRef nspace, ir::Name name);

	ir::NameRef push_scope(ir::NameRef nspace);
	void pop_scope();

	ir::NameRef root() const { return root_namespace; }
	ir::NameRef current() const { return namespace_stack[namespace_stack.size() - 1]; }

	std::vector<ir::Name>& name_table() { return names; }
};
} // namespace sema