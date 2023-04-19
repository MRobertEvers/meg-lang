#include "AstNode.h"

AstFuncProto::AstFuncProto(
	Linkage linkage,
	Routine routine,
	AstNode* id,
	std::vector<AstNode*> parameters,
	VarArg var_arg,
	AstNode* rt_type_declarator)
	: routine(routine)
	, linkage(linkage)
	, parameters(parameters)
	, id(id)
	, var_arg(var_arg)
	, rt_type_declarator(rt_type_declarator)
{}

std::string
to_string(NodeKind type)
{
	switch( type )
	{
	case NodeKind::Invalid:
		return "Invalid";
	case NodeKind::Module:
		return "Module";
	case NodeKind::Id:
		return "Id";
	case NodeKind::VarDecl:
		return "VarDecl";
	case NodeKind::Return:
		return "Return";
	case NodeKind::Block:
		return "Block";
	case NodeKind::TypeDeclarator:
		return "TypeDeclarator";
	case NodeKind::Func:
		return "Func";
	case NodeKind::FuncProto:
		return "FuncProto";
	case NodeKind::Expr:
		return "Expr";
	case NodeKind::Stmt:
		return "Stmt";
	case NodeKind::FuncCall:
		return "FuncCall";
	case NodeKind::Let:
		return "Let";
	case NodeKind::If:
		return "If";
	case NodeKind::SizeOf:
		return "SizeOf";
	case NodeKind::BoolLiteral:
		return "BoolLiteral";
	case NodeKind::BinOp:
		return "BinOp";
	case NodeKind::Struct:
		return "Struct";
	case NodeKind::Union:
		return "Union";
	case NodeKind::Enum:
		return "Enum";
	case NodeKind::NumberLiteral:
		return "NumberLiteral";
	case NodeKind::ArrayAccess:
		return "ArrayAccess";
	case NodeKind::AddressOf:
		return "AddressOf";
	case NodeKind::EnumMember:
		return "EnumMember";
	case NodeKind::MemberAccess:
		return "MemberAccess";
	case NodeKind::BoolNot:
		return "BoolNot";
	case NodeKind::Deref:
		return "Deref";
	case NodeKind::Case:
		return "Case";
	case NodeKind::Switch:
		return "Switch";
	case NodeKind::Break:
		return "Break";
	case NodeKind::Default:
		return "Default";
	case NodeKind::Continue:
		return "Continue";
	case NodeKind::Assign:
		return "Assign";
	case NodeKind::For:
		return "For";
	case NodeKind::While:
		return "While";
	case NodeKind::Is:
		return "Is";
	case NodeKind::DiscrimatingBlock:
		return "DiscrimatingBlock";
	case NodeKind::Interface:
		return "Interface";
	case NodeKind::Template:
		return "Template";
	case NodeKind::TemplateId:
		return "TemplateId";
	case NodeKind::Yield:
		return "Yield";
	case NodeKind::Using:
		return "Using";
	case NodeKind::Designator:
		return "Designator";
	case NodeKind::Initializer:
		return "Initializer";
	case NodeKind::StringLiteral:
		return "StringLiteral";
	}

	return "Bad";
}