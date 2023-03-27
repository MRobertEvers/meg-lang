#pragma once
#include "Name.h"
#include "QualifiedName.h"

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
		sema::NameRef name_;
	};

public:
	NameLookupResult()
		: kind(RNotFound)
	{}

	NameLookupResult(sema::NameRef name)
		: name_(name)
		, kind(RName)
	{}

	bool is_found() const { return kind != RNotFound; }

	sema::NameRef result() const
	{
		assert(is_found());
		return name_;
	}
};

class Lookup
{
	std::vector<sema::Name> names;

	sema::NameRef root_namespace;

	std::vector<sema::NameRef> namespace_stack;

public:
	Lookup();

	// Lookup starting from root.
	NameLookupResult lookup_fqn(QualifiedName const& name);
	// Lookup in the current namespace
	NameLookupResult lookup(QualifiedName const& name);
	// Lookup starting in nspace.
	NameLookupResult lookup(QualifiedName const& name, sema::NameRef nspace);

	sema::NameRef get(sema::NameId name_id);
	sema::NameRef add_name(sema::Name name);
	sema::NameRef add_name(sema::NameRef nspace, sema::Name name);

	sema::NameRef push_scope(sema::NameRef nspace);
	void pop_scope();

	sema::NameRef root() const { return root_namespace; }
	sema::NameRef current() const { return namespace_stack[namespace_stack.size() - 1]; }

	std::vector<sema::Name>& name_table() { return names; }
};
} // namespace sema