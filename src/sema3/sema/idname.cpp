#include "idname.h"

#include "ir/QualifiedName.h"
using namespace sema;

ir::QualifiedName
sema::idname(ast::AstList<std::string*>& name_parts)
{
	ir::QualifiedName name;

	for( auto name_part : &name_parts )
	{
		name.add_part(*name_part);
	}

	return name;
}

ir::QualifiedName
sema::idname(ast::AstId const& id)
{
	ir::QualifiedName name;

	for( auto name_part : id.name_parts )
	{
		name.add_part(*name_part);
	}

	return name;
}