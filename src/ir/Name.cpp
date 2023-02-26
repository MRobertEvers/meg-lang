#include "Name.h"

using namespace ir;

std::optional<NameId>
Name::parent()
{
	return this->parent_;
}

std::optional<NameId>
Name::lookup(std::string name)
{
	auto iter_lookup = lookup_.find(name);

	if( iter_lookup != lookup_.end() )
		return iter_lookup->second;
	else
		return std::optional<NameId>();
}

void
Name::add_name(std::string name, NameId id)
{
	lookup_.emplace(name, id);
}

std::optional<NameRef>
NameRef::lookup(std::string name_str) const
{
	NameRef current = *this;
	while( true )
	{
		Name& name = current.name();
		auto maybe_id = name.lookup(name_str);

		if( maybe_id.has_value() )
			return NameRef(names_, maybe_id.value());

		if( name.parent().has_value() )
			current = NameRef(names_, name.parent().value());
		else
			break;
	}

	return std::optional<NameRef>();
}

NameRef
NameRef::add_name(Name create_name)
{
	auto id_index = names_->size();
	auto name = names_->emplace_back(create_name);

	this->name().add_name(name.name_str(), id_index);

	return NameRef(names_, id_index);
}

std::string
NameRef::to_fqn_string() const
{
	std::string s;

	int ind = 0;

	NameRef current = *this;
	while( true )
	{
		Name& name = current.name();

		// Break at the root namespace.
		if( !name.parent().has_value() )
			break;

		if( ind != 0 )
			s += "__";
		s += name.name_str();

		current = NameRef(names_, name.parent().value());

		ind += 1;
	}

	return s;
}