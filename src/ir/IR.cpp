#include "IR.h"

using namespace ir;

BasicBlock::BasicBlock()
{}

Alloca::Alloca(TypeInstance type)
	: type(type)
	, Inst(InstKind::Alloca)
{}

Return::Return(Inst* operand)
	: operand(operand)
	, Inst(InstKind::Return)
{}

FnDecl::FnDecl(TypeInstance type)
	: type(type)
	, Inst(InstKind::FnDecl)
{}

Function::Function(TypeInstance type)
	: type(type)
	, Inst(InstKind::Function)
{}

ConstInt::ConstInt(unsigned long long val)
	: value(val)
	, Inst(InstKind::ConstInt)
{}