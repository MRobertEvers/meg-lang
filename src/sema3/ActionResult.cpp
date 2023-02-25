#include "ActionResult.h"

using namespace ir;

ActionResult::ActionResult(LValue lvalue)
	: lvalue_(lvalue)
	, kind(Kind::LValue)
{}

ActionResult::ActionResult(RValue rvalue)
	: rvalue_(rvalue)
	, kind(Kind::RValue)
{}

ActionResult::ActionResult(sema::TypeInstance type)
	: type_(type)
	, kind(Kind::RValue)
{}

ActionResult::ActionResult()
	: kind(Kind::Void)
{}