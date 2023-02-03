#include "AstNode.h"
using namespace ast;

String
ast::to_string(NodeType type)
{
	switch( type )
	{
	case NodeType::Invalid:
		return "Invalid";
	case NodeType::Module:
		return "Module";
	case NodeType::Fn:
		return "Fn";
	case NodeType::FnProto:
		return "FnProto";
	case NodeType::FnParamList:
		return "FnParamList";
	case NodeType::ValueDecl:
		return "ValueDecl";
	case NodeType::FnCall:
		return "FnCall";
	case NodeType::ExprList:
		return "ExprList";
	case NodeType::Block:
		return "Block";
	case NodeType::BinOp:
		return "BinOp";
	case NodeType::Id:
		return "Id";
	case NodeType::Assign:
		return "Assign";
	case NodeType::If:
		return "If";
	case NodeType::Let:
		return "Let";
	case NodeType::Return:
		return "Return";
	case NodeType::Struct:
		return "Struct";
	case NodeType::MemberDef:
		return "MemberDef";
	case NodeType::While:
		return "While";
	case NodeType::For:
		return "For";
	case NodeType::StringLiteral:
		return "StringLiteral";
	case NodeType::NumberLiteral:
		return "NumberLiteral";
	case NodeType::TypeDeclarator:
		return "TypeDeclarator";
	case NodeType::MemberAccess:
		return "MemberAccess";
	case NodeType::Expr:
		return "Expr";
	case NodeType::Stmt:
		return "Stmt";
	}
}