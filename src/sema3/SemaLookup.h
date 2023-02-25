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

public:
	SemaLookup();

	NameLookupResult lookup(QualifiedName const& name);

	void add_name(ir::NameRef nspace, ir::Name name);

	ir::NameRef root() const { return root_namespace; }
};
} // namespace sema