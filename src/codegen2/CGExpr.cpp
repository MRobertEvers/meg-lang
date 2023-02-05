#include "CGExpr.h"
using namespace cg;

llvm::Value*
CGExpr::as_value()
{
	switch( type )
	{
	case CGExprType::Value:
		return data.value;
	case CGExprType::FunctionValue:
		return data.fn;
	default:
		return nullptr;
	}
}