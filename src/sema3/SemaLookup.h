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
		NameRef name_;
	};

public:
	NameLookupResult()
		: kind(RNotFound)
	{}

	NameLookupResult(NameRef name)
		: name_(name)
		, kind(RName)
	{}

	bool is_found() const { return kind != RNotFound; }

	NameRef result() const
	{
		assert(is_found());
		return name_;
	}
};

class SemaLookup
{
	std::vector<Name> names;

	NameRef root_namespace;

public:
	SemaLookup();

	NameLookupResult lookup(QualifiedName const& name);

	void add_name(NameRef nspace, Name name);

	NameRef root() const { return root_namespace; }
};
} // namespace sema