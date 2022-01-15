

#include "Type.h"

using namespace hir;

Type const*
Type::get_member_type(String const& name) const
{
	auto member = members.find(name);
	if( member != members.cend() )
	{
		return member->second;
	}
	else
	{
		return nullptr;
	}
}

void
Type::add_member(String const& name, Type const& type)
{
	members.insert(std::make_pair(name, &type));
}