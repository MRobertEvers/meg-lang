#include "lower_for.h"

#include "../Sema2.h"
#include "common/Vec.h"
using namespace sema;

SemaResult<ir::IRBlock*>
sema::lower_for(Sema2& sema, ir::IRFor* ir_for)
{
	Vec<ir::IRStmt*> vec;
	vec.push_back(sema.Stmt(ir_for));
	return sema.Block(ir_for->node, vec);
}