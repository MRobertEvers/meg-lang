#include "codegen_string_literal.h"

#include "../Codegen.h"
#include "common/String.h"

using namespace cg;

static char
escape_char(char c)
{
	// \a	07	Alert (Beep, Bell) (added in C89)[1]
	// \b	08	Backspace
	// \e	1B	Escape character
	// \f	0C	Formfeed Page Break
	// \n	0A	Newline (Line Feed); see notes below
	// \r	0D	Carriage Return
	// \t	09	Horizontal Tab
	// \v	0B	Vertical Tab
	// \\	5C	Backslash
	// \'	27	Apostrophe or single quotation mark
	// \"	22	Double quotation mark
	// \?	3F	Question mark (used to avoid trigraphs)

	switch( c )
	{
	case 'a':
		return 0x07;
	case 'b':
		return 0x08;
	case 'e':
		return 0x1B;
	case 'f':
		return 0x0C;
	case 'n':
		return 0x0A;
	case 'r':
		return 0x0D;
	case 't':
		return 0x09;
	case 'v':
		return 0x0B;
	case '\\':
		return '\\';
	case '\'':
		return '\'';
	case '"':
		return '"';
	case '?':
		return '?';
	default:
		return c;
	}
}

static String
escape_string(String s)
{
	String res;
	res.reserve(s.size());
	bool escape = false;
	for( auto c : s )
	{
		if( !escape && c == '\\' )
		{
			escape = true;
			continue;
		}

		res.push_back(escape ? escape_char(c) : c);
		escape = false;
	}

	return res;
}

CGResult<CGExpr>
cg::codegen_string_literal(CG& codegen, ir::IRStringLiteral* lit)
{
	//

	auto llvm_literal = llvm::ConstantDataArray::getString(
		*codegen.Context, escape_string(*lit->value).c_str(), true);

	llvm::GlobalVariable* llvm_global = new llvm::GlobalVariable(
		*codegen.Module,
		llvm_literal->getType(),
		true,
		llvm::GlobalValue::InternalLinkage,
		llvm_literal);
	llvm::Constant* zero =
		llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(*codegen.Context));
	llvm::Constant* indices[] = {zero, zero};
	llvm::Constant* llvm_str =
		llvm::ConstantExpr::getGetElementPtr(llvm_literal->getType(), llvm_global, indices);

	return CGExpr::MakeRValue(RValue(llvm_str));
}
