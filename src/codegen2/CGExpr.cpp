#include "CGExpr.h"
using namespace cg;

CGExpr
CGExpr::MakeAddress(LLVMAddress addr)
{
	return CGExpr(addr);
}

CGExpr
CGExpr::MakeRValue(RValue addr)
{
	return CGExpr(addr);
}
