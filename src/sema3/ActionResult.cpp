#include "ActionResult.h"

using namespace ir;

ActionResult::ActionResult(Action rvalue)
	: action_(rvalue)
	, kind(Kind::Action)
{}

ActionResult::ActionResult(ir::TypeInstance type)
	: type_(type)
	, kind(Kind::Type)
{}

ActionResult::ActionResult()
	: kind(Kind::Void)
{}