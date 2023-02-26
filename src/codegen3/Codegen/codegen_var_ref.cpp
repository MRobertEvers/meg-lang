#include "codegen_var_ref.h"

#include "../Codegen.h"
#include "lookup.h"

using namespace cg;

CGExpr
cg::codegen_var_ref(CG& codegen, ir::VarRef* var_ref)
{
	// auto value = get_value(*this, to_single_name(id->name));
	// if( value.has_value() )
	// 	return CGExpr::MakeAddress(value.value().address());

	auto value = codegen.vars.find(var_ref->name_id.index());
	assert(value != codegen.vars.end());

	return CGExpr::MakeAddress(value->second);

	// TODO: This supports 'let my_point = Point' initialization.
	// I can see the "Codegen Id" function getting a little wild.
	// Need to rethink it.
	// TODO: This should be get type by name...
	// Otherwise bad codegen.
	// auto maybe_type = get_type(*this, id->type_instance);
	// if( maybe_type.ok() )
	// 	return CGExpr();

	// return CGError("Undeclared identifier! " + to_single_name(id->name));
}