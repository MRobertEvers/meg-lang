#include "idname.h"

#include "../QualifiedName.h"
using namespace sema;

QualifiedName
sema::idname(ast::AstList<std::string*>& name_parts)
{
	QualifiedName name;

	for( auto name_part : &name_parts )
	{
		name.add_part(*name_part);
	}

	return name;
}

QualifiedName
sema::idname(ast::AstId const& id)
{
	QualifiedName name;

	for( auto name_part : id.name_parts )
	{
		name.add_part(*name_part);
	}

	return name;
}