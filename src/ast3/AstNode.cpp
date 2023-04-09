#include "AstNode.h"

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
	case NodeKind::Deref:
		return "Deref";
	}

	return "Bad";
}