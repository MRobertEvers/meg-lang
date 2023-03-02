#include <llvm/IR/IRBuilder.h>

namespace cg
{

/**
 * @brief Pass by value still uses pointers.
 * The difference is that by-value structs are copied to their own
 * memory location before the call and a pointer to that is passed in.
 *
 */
struct LLVMArgABIInfo
{
	enum Kind : char
	{
		Default,
		SRet,
		Value,

		// Indicates that this ABI Arg Info contains
		// no information about the arg becuase it was
		// provided as a var arg. User must check
		// the type of the argument by examining
		// the expression type.
		UncheckedVarArg
	};

	Kind attr = Default;
	llvm::Type* llvm_type;

	LLVMArgABIInfo(Kind attr, llvm::Type* llvm_type)
		: attr(attr)
		, llvm_type(llvm_type){};

	bool is_sret() const { return attr == SRet; }

	static LLVMArgABIInfo Unchecked()
	{
		return LLVMArgABIInfo(LLVMArgABIInfo::UncheckedVarArg, nullptr);
	}
};

} // namespace cg