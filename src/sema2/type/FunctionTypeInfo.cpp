#include "FunctionTypeInfo.h"

using namespace sema;

FunctionTypeInfo::FunctionTypeInfo(Vec<TypedMember> args, TypeInstance rt)
	: members_order(args)
	, return_type(rt)
{
	for( auto arg : args )
	{
		members.emplace(arg.name, arg);
	}
}