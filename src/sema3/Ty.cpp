#include "Ty.h"

static int
kind_width(TyInt::IntKind kind)
{
	switch( kind )
	{
	case TyInt::IntKind::i8:
	case TyInt::IntKind::u8:
		return 8;
	case TyInt::IntKind::i16:
	case TyInt::IntKind::u16:
		return 16;
	case TyInt::IntKind::i32:
	case TyInt::IntKind::u32:
		return 32;
	case TyInt::IntKind::i64:
	case TyInt::IntKind::u64:
		return 64;
	}
}

static TyInt::Sign
kind_sign(TyInt::IntKind kind)
{
	switch( kind )
	{
	case TyInt::IntKind::i8:
	case TyInt::IntKind::i16:
	case TyInt::IntKind::i32:
	case TyInt::IntKind::i64:
		return TyInt::Sign::Signed;
	case TyInt::IntKind::u8:
	case TyInt::IntKind::u16:
	case TyInt::IntKind::u32:
	case TyInt::IntKind::u64:
		return TyInt::Sign::Unsigned;
	}
}

TyInt::TyInt(std::string name, IntKind kind)
	: name(name)
	, kind(kind)
	, width(kind_width(kind))
	, sign(kind_sign(kind)){};