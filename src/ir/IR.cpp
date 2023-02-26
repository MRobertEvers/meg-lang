#include "IR.h"

using namespace ir;

BasicBlock::BasicBlock()
{}

Alloca::Alloca(NameId name_id, TypeInstance type)
	: type(type)
	, name_id(name_id)
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

Store::Store(Inst* lhs, Inst* rhs)
	: lhs(lhs)
	, rhs(rhs)
	, Inst(InstKind::Store)
{}

VarRef::VarRef(NameId name_id)
	: name_id(name_id)
	, Inst(InstKind::VarRef)
{}