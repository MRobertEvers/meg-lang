#include "codegen_enum.h"

#include "../Codegen.h"
#include "lookup.h"
using namespace cg;

/**
 * @brief
 *  For enums, need to look up name,
 * and have a mapping of member to llvm type
 *
 * Defines a struct, and a union
 *
 * struct MyEnum {
 * 	int type;
 * 	union {
 * 		...
 * 	}
 * }
 * @param st
 * @return CGResult<CGExpr>
 */
CGResult<CGExpr>
cg::codegen_enum(CG& codegen, ir::IREnum* st)
{
	llvm::Type* llvm_max_type_by_size = nullptr;
	int max_size = 0;

	llvm::Type* llvm_current_type = nullptr;
	int current_type_size = 0;
	for( auto& member : st->members )
	{
		auto enum_member = member.second;

		if( enum_member->kind == ir::IREnumMember::Kind::Struct )
		{
			auto cg = codegen.codegen_struct(enum_member->struct_member);
			if( !cg.ok() )
				return cg;

			auto typer = get_type(
				codegen, sema::TypeInstance::OfType(enum_member->struct_member->struct_type));
			if( !typer.ok() )
				return typer;
			llvm_current_type = typer.unwrap();
			auto llvm_size = codegen.Module->getDataLayout().getTypeAllocSize(llvm_current_type);
			current_type_size = llvm_size.getKnownMinSize();
		}
		else
		{
			// Empty fields are 0.
		}

		if( current_type_size > max_size )
		{
			llvm_max_type_by_size = llvm_current_type;
			max_size = current_type_size;
		}
	}
	std::vector<llvm::Type*> members = {llvm::Type::getInt32Ty(*codegen.Context)};
	if( llvm_max_type_by_size != nullptr )
		members.push_back(llvm_max_type_by_size);

	auto enum_type = st->enum_type;
	std::string name_str = st->name.to_fqn_string();
	llvm::StructType* llvm_struct_type =
		llvm::StructType::create(*codegen.Context, members, name_str);

	codegen.types.emplace(enum_type, llvm_struct_type);

	return CGExpr();
}
