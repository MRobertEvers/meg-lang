#include "lookup.h"

#include "../Codegen.h"
#include "CGFunctionContext.h"

using namespace cg;

CGResult<llvm::Type*>
cg::get_base_type(CG& cg, sema::TypeInstance ty)
{
	auto maybe_llvm_type = cg.find_type(ty.type);
	if( !maybe_llvm_type.has_value() )
		return CGError("Missing Type!");

	return maybe_llvm_type.value();
}

CGResult<llvm::Type*>
cg::get_type(CG& cg, sema::TypeInstance ty)
{
	auto maybe_llvm_type = get_base_type(cg, ty);
	if( !maybe_llvm_type.ok() )
		return maybe_llvm_type;

	auto type = maybe_llvm_type.unwrap();

	// TODO: Try to use opaque pointers? We have to keep track of the pointer
	// type ourselves.
	for( int i = 0; i < ty.indirection_level; i++ )
		type = type->getPointerTo();

	return type;
}

CGResult<llvm::Type*>
cg::get_type(CG& cg, ir::IRTypeDeclaraor* decl)
{
	return get_type(cg, decl->type_instance);
}

CGResult<llvm::Type*>
cg::get_type(CG& cg, ir::IRValueDecl* decl)
{
	return get_type(cg, decl->type_decl);
}

CGResult<get_params_types_t>
cg::get_params_types(CG& cg, ir::IRProto* proto)
{
	get_params_types_t args;
	args.is_var_arg = false;

	for( auto& arg : *proto->args )
	{
		if( arg->type == ir::IRParamType::ValueDecl )
		{
			auto value_decl = arg->data.value_decl;
			auto argsr = get_type(cg, value_decl);
			if( !argsr.ok() )
				return argsr;

			auto type = value_decl->type_decl->type_instance;
			auto ArgType = argsr.unwrap();
			if( type.type->is_struct_type() )
			{
				args.args.push_back(ArgumentType(ArgumentAttr::Value, ArgType));
			}
			else
			{
				args.args.push_back(ArgumentType(ArgumentAttr::Default, ArgType));
			}
		}
		else
		{
			args.is_var_arg = true;
		}
	}

	return args;
}

// Get LValue?
std::optional<CGExpr>
cg::get_value(CG& cg, String const& name)
{
	auto iter_value = cg.values.find(name);
	if( iter_value != cg.values.end() )
		return iter_value->second;
	else
		return std::optional<CGExpr>();
}