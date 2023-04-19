#include "Token.h"

TokenView::TokenView()
{}

TokenView::TokenView(char const* start)
	: start(start)
	, size(0)
{}

Token::Token(){};
Token::Token(TokenView view, TokenKind kind)
	: view(view)
	, kind(kind)
{}

char const*
get_tokentype_string(TokenKind const& tok_kind)
{
	switch( tok_kind )
	{
	case TokenKind::YieldKw:
		return "YieldKw";
	case TokenKind::AsyncKw:
		return "AsyncKw";
	case TokenKind::ImplKw:
		return "ImplKw";
	case TokenKind::Identifier:
		return "Identifier";
	case TokenKind::NumberLiteral:
		return "NumberLiteral";
	case TokenKind::StringLiteral:
		return "StringLiteral";
	case TokenKind::LineComment:
		return "LineComment";
	case TokenKind::OpenSquare:
		return "OpenSquare";
	case TokenKind::CloseSquare:
		return "CloseSquare";
	case TokenKind::OpenParen:
		return "OpenParen";
	case TokenKind::CloseParen:
		return "CloseParen";
	case TokenKind::OpenCurly:
		return "OpenCurly";
	case TokenKind::CloseCurly:
		return "CloseCurly";
	case TokenKind::Colon:
		return "Colon";
	case TokenKind::SemiColon:
		return "SemiColon";
	case TokenKind::Star:
		return "Star";
	case TokenKind::Slash:
		return "Slash";
	case TokenKind::Plus:
		return "Plus";
	case TokenKind::Minus:
		return "Minus";
	case TokenKind::Eq:
		return "Eq";
	case TokenKind::FnKw:
		return "FnKw";
	case TokenKind::ReturnKw:
		return "ReturnKw";
	case TokenKind::LetKw:
		return "LetKw";
	case TokenKind::IfKw:
		return "IfKw";
	case TokenKind::ElseKw:
		return "ElseKw";
	case TokenKind::Gt:
		return "Gt";
	case TokenKind::Lt:
		return "Lt";
	case TokenKind::GtEq:
		return "GtEq";
	case TokenKind::LtEq:
		return "LtEq";
	case TokenKind::Bad:
		return "Bad";
	case TokenKind::Comma:
		return "Comma";
	case TokenKind::ColonColon:
		return "ColonColon";
	case TokenKind::TrueKw:
		return "TrueKw";
	case TokenKind::FalseKw:
		return "FalseKw";
	case TokenKind::StructKw:
		return "StructKw";
	case TokenKind::UnionKw:
		return "UnionKw";
	case TokenKind::EnumKw:
		return "EnumKw";
	case TokenKind::SizeOfKw:
		return "SizeOf";
	case TokenKind::EqEq:
		return "EqEq";
	case TokenKind::Ampersand:
		return "Ampersand";
	case TokenKind::Dot:
		return "Dot";
	case TokenKind::IsKw:
		return "IsKw";
	case TokenKind::ForKw:
		return "ForKw";
	case TokenKind::WhileKw:
		return "WhileKw";
	case TokenKind::ExternKw:
		return "ExternKw";
	case TokenKind::BreakKw:
		return "BreakKw";
	case TokenKind::DefaultKw:
		return "DefaultKw";
	case TokenKind::ContinueKw:
		return "ContinueKw";
	case TokenKind::CaseKw:
		return "CaseKw";
	case TokenKind::SkinnyArrow:
		return "SkinnyArrow";
	case TokenKind::FatArrow:
		return "FatArrow";
	case TokenKind::Exclam:
		return "Exclam";
	case TokenKind::ExclamEq:
		return "ExclamEq";
	case TokenKind::SwitchKw:
		return "SwitchKw";
	case TokenKind::TemplateKw:
		return "TemplateKw";
	case TokenKind::TypenameKw:
		return "TypenameKw";
	case TokenKind::AmpAmp:
		return "AmpAmp";
	case TokenKind::Pipe:
		return "Pipe";
	case TokenKind::PipePipe:
		return "PipePipe";
	case TokenKind::InterfaceKw:
		return "InterfaceKw";
	case TokenKind::Caret:
		return "Caret";
	case TokenKind::Tilde:
		return "Tilde";
	case TokenKind::Percent:
		return "Percent";
	case TokenKind::UsingKw:
		return "UsingKw";
	case TokenKind::GtGt:
		return "GtGt";
	case TokenKind::LtLt:
		return "LtLt";
	case TokenKind::Ellipsis:
		return "Ellipsis";
	case TokenKind::Eof:
		return "Eof";
	}

	return "Unk";
}