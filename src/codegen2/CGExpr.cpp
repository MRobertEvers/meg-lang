#include "CGExpr.h"
using namespace cg;

void
CGExpr::add_discrimination(CGExpr expr)
{
	discriminations.push_back(expr);
}

CGExpr
CGExpr::get_discrimination(int ind)
{
	return discriminations.at(ind);
}

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
