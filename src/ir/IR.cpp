#include "IR.h"

using namespace ir;

FnDecl::FnDecl(sema::NameRef name, Linkage linkage)
	: name(name)
	, linkage(linkage)
{}