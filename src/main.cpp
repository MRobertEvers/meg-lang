

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>

// clang-format off
#define CHAR_WHITESPACE_CASES \
    ' ': \ 
    case '\n': \
    case '\r': \
    case '\t'

#define CHAR_DIGIT_CASES \
         '0': \
    case '1': \
    case '2': \
    case '3': \
    case '4': \
    case '5': \
    case '6': \
    case '7': \
    case '8': \
    case '9'

#define CHAR_IDENTIFIER_START_CASES \
         'a': \
    case 'b': \
    case 'c': \
    case 'd': \
    case 'e': \
    case 'f': \
    case 'g': \
    case 'h': \
    case 'i': \
    case 'j': \
    case 'k': \
    case 'l': \
    case 'm': \
    case 'n': \
    case 'o': \
    case 'p': \
    case 'q': \
    case 'r': \
    case 's': \
    case 't': \
    case 'u': \
    case 'v': \
    case 'w': \
    case 'x': \
    case 'y': \
    case 'z': \
    case 'A': \
    case 'B': \
    case 'C': \
    case 'D': \
    case 'E': \
    case 'F': \
    case 'G': \
    case 'H': \
    case 'I': \
    case 'J': \
    case 'K': \
    case 'L': \
    case 'M': \
    case 'N': \
    case 'O': \
    case 'P': \
    case 'Q': \
    case 'R': \
    case 'S': \
    case 'T': \
    case 'U': \
    case 'V': \
    case 'W': \
    case 'X': \
    case 'Y': \
    case 'Z': \
    case '_'

#define CHAR_IDENTIFIER_CASES \
    CHAR_IDENTIFIER_START_CASES: \
    case CHAR_DIGIT_CASES

// clang-format on

enum class TokenType
{
	// Keyword or other identifier
	identifier,
	// Int or string
	literal,

	// One character tokens
	star,
	plus,
	minus,
	slash,

	open_paren,
	close_paren,
	open_curly,
	close_curly,

	semicolon,

	// Keyword
	fn,

	bad
};

enum class LiteralType
{
	integer,
	floating,
	string,
	string_template,
	none
};

struct Token
{
	TokenType type = TokenType::bad;
	LiteralType literal_type = LiteralType::none;
	char const* start = nullptr;
	int size = 0;
};

class Lexer
{
public:
	Lexer(char const* in)
		: input_(in)
		, cursor_(0)
		, input_len_(strlen(in))
	{}

	std::vector<Token> lex();

private:
	char const* input_;
	int cursor_;
	int input_len_;

	Token lex_consume_identifier();
	Token lex_consume_number();

	Token lex_consume_single();

public:
	static void print_tokens(std::vector<Token> const& tokens)
	{
		for( auto& tok : tokens )
		{
			std::string sz{tok.start, tok.start + tok.size};
			std::cout << sz << std::endl;
		}
	}
};

std::vector<Token>
Lexer::lex()
{
	std::vector<Token> result{};
	result.reserve(30);
	cursor_ = 0;

	for( ; cursor_ < input_len_; cursor_++ )
	{
		char c = input_[cursor_];

		switch( c )
		{
		case CHAR_WHITESPACE_CASES:

			break;

		case CHAR_IDENTIFIER_START_CASES:
			result.push_back(lex_consume_identifier());
			break;

		case CHAR_DIGIT_CASES:
			result.push_back(lex_consume_number());
			break;

		case '*':
		case '+':
		case '-':
		case '/':
		case '(':
		case ')':
		case '{':
		case '}':
		case ';':
			result.push_back(lex_consume_single());
			break;

		default:
			std::cout << "Unexpected character: '" << c << "'" << std::endl;
			break;
		}
	}

	return result;
}

struct KeywordIdentifierTuple
{
	char const* keyword;
	TokenType type;
};

// clang-format off
KeywordIdentifierTuple keywords[] =
{
	// clang-format on
	{"fn", TokenType::fn}
	// clang-format off
};
// clang-format on

TokenType
get_identifier_or_keyword_type(Token const& token)
{
	for( auto& keyword : keywords )
	{
		if( strncmp(keyword.keyword, token.start, token.size) == 0 )
		{
			return keyword.type;
		}
	}

	return TokenType::identifier;
}

Token
Lexer::lex_consume_identifier()
{
	Token token{};
	token.start = &input_[cursor_];
	for( ; cursor_ < input_len_; cursor_++ )
	{
		char c = input_[cursor_];

		switch( c )
		{
		case CHAR_IDENTIFIER_CASES:
			token.size += 1;
			break;

		default:
			goto done;
			break;
		}
	}
done:
	token.type = get_identifier_or_keyword_type(token);

	cursor_--;

	return token;
}

Token
Lexer::lex_consume_number()
{
	Token token{};
	token.start = &input_[cursor_];
	token.literal_type = LiteralType::integer;
	token.type = TokenType::literal;
	for( ; cursor_ < input_len_; cursor_++ )
	{
		char c = input_[cursor_];

		switch( c )
		{
		case CHAR_DIGIT_CASES:
			token.size += 1;
			break;

		default:
			goto done;
			break;
		}
	}
done:
	cursor_--;
	return token;
}

Token
Lexer::lex_consume_single()
{
	Token token{};
	token.start = &input_[cursor_];
	token.size = 1;

	char c = input_[cursor_];

	switch( c )
	{
	case '*':
		token.type = TokenType::star;
		break;
	case '+':
		token.type = TokenType::plus;
		break;
	case '-':
		token.type = TokenType::minus;
		break;
	case '/':
		token.type = TokenType::slash;
		break;
	case '(':
		token.type = TokenType::open_paren;
		break;
	case ')':
		token.type = TokenType::close_paren;
		break;
	case '{':
		token.type = TokenType::open_curly;
		break;
	case '}':
		token.type = TokenType::close_curly;
		break;
	case ';':
		token.type = TokenType::semicolon;
		break;
	default:
		token.type = TokenType::bad;
		break;
	}

	return token;
}

class ASTFunction
{
public:
};

void
codegen_fn()
{}

void
codegen(std::vector<Token> const& tokens)
{
	for( auto& token : tokens )
	{
		switch( token.type )
		{
		case TokenType::fn:

			break;

		default:
			break;
		}
	}
}

using namespace llvm;
static LLVMContext TheContext;

#include <iostream>
int
main()
{
	char const buf[] = "fn func() { return 9*8; } fn main() { return func(); }";
	Lexer lex{buf};

	auto tokens = lex.lex();

	Lexer::print_tokens(tokens);

	codegen(tokens);

	IRBuilder<> Builder{TheContext};
	Module Mod{"main", TheContext};
	FunctionType* FT = FunctionType::get(Type::getDoubleTy(TheContext), false);
	Function* Func = Function::Create(FT, Function::ExternalLinkage, "floater", Mod);

	BasicBlock* Block = BasicBlock::Create(TheContext, "entry", Func);
	Builder.SetInsertPoint(Block);

	auto const L = ConstantFP::get(TheContext, APFloat(4.5));
	auto const R = ConstantFP::get(TheContext, APFloat(5.9));

	Value* ResVar = Builder.CreateFAdd(L, R, "addtmp");

	Builder.CreateRet(ResVar);

	verifyFunction(*Func);

	std::string Str;
	raw_string_ostream OS(Str);

	Mod.print(OS, nullptr);

	std::cout << Str;

	auto CPU = "generic";
	auto Features = "";

	InitializeNativeTarget();
	InitializeNativeTargetAsmParser();
	InitializeNativeTargetAsmPrinter();
	TargetOptions opt;
	auto RM = Optional<Reloc::Model>();

	// auto TargetTriple = sys::getDefaultTargetTriple();
	std::string TargetTriple = "aarch64-app-darwin";
	std::cout << "Target: " << TargetTriple << std::endl;

	std::string Error;
	auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);
	if( !Target )
	{
		errs() << Error;
		return 1;
	}

	auto TheTargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

	Mod.setDataLayout(TheTargetMachine->createDataLayout());
	Mod.setTargetTriple(TargetTriple);

	auto Filename = "output.o";
	std::error_code EC;
	raw_fd_ostream dest(Filename, EC, sys::fs::OF_None);

	if( EC )
	{
		errs() << "Could not open file: " << EC.message();
		return 1;
	}

	legacy::PassManager pass;
	auto FileType = CGFT_ObjectFile;

	if( TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType) )
	{
		errs() << "TheTargetMachine can't emit a file of this type";
		return 1;
	}

	pass.run(Mod);
	dest.flush();

	outs() << "Wrote " << Filename << "\n";

	return 0;
}