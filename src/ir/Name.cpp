#include "Name.h"

using namespace ir;

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
	Name& name = this->name();
	auto maybe_id = name.lookup(name_str);

	if( !maybe_id.has_value() )
		return std::optional<NameRef>();

	return NameRef(names_, maybe_id.value());
}

NameRef
NameRef::add_name(Name create_name)
{
	auto id_index = names_->size();
	auto name = names_->emplace_back(create_name);

	this->name().add_name(name.name_str(), id_index);

	return NameRef(names_, id_index);
}
