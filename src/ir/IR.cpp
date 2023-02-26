#include "IR.h"

using namespace ir;

BasicBlock::BasicBlock()
{}

Alloca::Alloca(NameId name_id, TypeInstance type)
	: name_id(name_id)
	, Inst(InstKind::Alloca, type)
{}

Return::Return(Inst* operand)
	: operand(operand)
	, Inst(InstKind::Return)
{}

FnDecl::FnDecl(NameId name_id, TypeInstance type)
	: name_id(name_id)
	, Inst(InstKind::FnDecl, type)
{}

FnDef::FnDef(std::vector<NameId> args, TypeInstance type)
	: args(args)
	, Inst(InstKind::FnDef, type)
{}

ConstInt::ConstInt(unsigned long long val)
	: value(val)
	, Inst(InstKind::ConstInt)
{}

StringLiteral::StringLiteral(std::string val)
	: value(val)
	, Inst(InstKind::StringLiteral)
{}

Store::Store(Inst* lhs, Inst* rhs)
	: lhs(lhs)
	, rhs(rhs)
	, Inst(InstKind::Store)
{}

VarRef::VarRef(NameId name_id, TypeInstance type)
	: name_id(name_id)
	, Inst(InstKind::VarRef, type)
{}

FnCall::FnCall(Inst* call_target, std::vector<Inst*> args, TypeInstance type)
	: call_target(call_target)
	, args(args)
	, Inst(InstKind::FnCall, type)
{}

Val::Val(NameId name_id, TypeInstance type)
	: name_id(name_id)
	, Inst(InstKind::Val, type)
{}

BinOp::BinOp(Inst* lhs, Inst* rhs, TypeInstance type)
	: lhs(lhs)
	, rhs(rhs)
	, Inst(InstKind::BinOp, type)
{}