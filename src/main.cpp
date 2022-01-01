

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
#include <map>
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

/// BinopPrecedence - This holds the precedence for each binary operator that is
/// defined.

using namespace llvm;
static std::map<char, int> BinopPrecedence;
static std::unique_ptr<LLVMContext> TheContext;
static std::unique_ptr<Module> TheModule;
static std::unique_ptr<IRBuilder<>> Builder;

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
	colon,

	// Keyword
	fn,
	return_keyword,

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
	unsigned int size = 0;
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
		case ':':
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
	{"fn", TokenType::fn},
	{"return", TokenType::return_keyword}
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
	case ':':
		token.type = TokenType::colon;
		break;
	default:
		token.type = TokenType::bad;
		break;
	}

	return token;
}

// Base class for all expressions
class ExprAST
{
public:
	virtual ~ExprAST() = default;

	virtual Value* codegen() = 0;
};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST
{
	int Val;

public:
	NumberExprAST(int Val)
		: Val(Val)
	{}

	Value* codegen() override { return ConstantInt::get(*TheContext, APInt(32, Val, true)); }
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST
{
	char Op;
	std::unique_ptr<ExprAST> LHS, RHS;

public:
	BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS)
		: Op(Op)
		, LHS(std::move(LHS))
		, RHS(std::move(RHS))
	{}

	Value* codegen() override
	{
		Value* L = LHS->codegen();
		Value* R = RHS->codegen();
		if( !L || !R )
			return nullptr;

		switch( Op )
		{
		case '+':
			return Builder->CreateAdd(L, R, "addtmp");
		case '-':
			return Builder->CreateSub(L, R, "subtmp");
		case '*':
			return Builder->CreateMul(L, R, "multmp");
		case '/':
			return Builder->CreateSDiv(L, R, "divtmp");
		case '<':
			L = Builder->CreateICmpULT(L, R, "cmptmp");
			// Convert bool 0/1 to double 0.0 or 1.0
			return L;
		default:
			return nullptr;
		}
	}
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST
{
	std::string Callee;
	std::vector<std::unique_ptr<ExprAST>> Args;

public:
	CallExprAST(const std::string& Callee, std::vector<std::unique_ptr<ExprAST>> Args)
		: Callee(Callee)
		, Args(std::move(Args))
	{}

	Value* codegen() override {}
};

/// CallExprAST - Expression class for function calls.
class ReturnExprAST : public ExprAST
{
	std::unique_ptr<ExprAST> Return;

public:
	ReturnExprAST(std::unique_ptr<ExprAST> Return)
		: Return(std::move(Return))
	{}

	Value* codegen() override
	{
		auto RetVal = Return->codegen();
		// Finish off the function.
		Builder->CreateRet(RetVal);

		return nullptr;
		return Constant::getNullValue(Type::getVoidTy(*TheContext));
	}
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class PrototypeAST
{
	std::string Name;
	std::vector<std::string> Args;

public:
	PrototypeAST(const std::string& Name, std::vector<std::string> Args)
		: Name(Name)
		, Args(std::move(Args))
	{}

	Function* codegen()
	{
		// Make the function type:  double(double,double) etc.
		std::vector<Type*> Doubles(Args.size(), Type::getDoubleTy(*TheContext));
		FunctionType* FT = FunctionType::get(Type::getDoubleTy(*TheContext), Doubles, false);

		Function* F = Function::Create(FT, Function::ExternalLinkage, Name, TheModule.get());

		// Set names for all arguments.
		unsigned Idx = 0;
		for( auto& Arg : F->args() )
			Arg.setName(Args[Idx++]);

		return F;
	}
	const std::string& getName() const { return Name; }
};

class BlockAST : public ExprAST
{
	std::vector<std::unique_ptr<ExprAST>> stmts;

public:
	BlockAST(std::vector<std::unique_ptr<ExprAST>> stmts)
		: stmts(std::move(stmts))
	{}

	Value* codegen()
	{
		for( auto& stmt : stmts )
		{
			stmt->codegen();
		}

		return nullptr;
		return Constant::getNullValue(Type::getVoidTy(*TheContext));
	}
};
/// FunctionAST - This class represents a function definition itself.
class FunctionAST
{
	std::unique_ptr<PrototypeAST> Proto;
	std::unique_ptr<ExprAST> Body;

public:
	FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<ExprAST> Body)
		: Proto(std::move(Proto))
		, Body(std::move(Body))
	{}

	Function* codegen()
	{
		// Transfer ownership of the prototype to the FunctionProtos map, but keep a
		// reference to it for use below.
		Function* TheFunction = Proto->codegen();
		if( !TheFunction )
			return nullptr;

		// Create a new basic block to start insertion into.
		BasicBlock* BB = BasicBlock::Create(*TheContext, "entry", TheFunction);
		Builder->SetInsertPoint(BB);

		Body->codegen();

		return TheFunction;
	}
};

class TokenCursor
{
	std::vector<Token> const& _tokens;
	int _index;

public:
	TokenCursor(std::vector<Token> const& toks)
		: _tokens(toks)
		, _index(0){};

	Token peek()
	{
		if( _index >= _tokens.size() )
		{
			throw new std::string{"oops"};
		}
		return _tokens[_index];
	}

	void adv()
	{
		_index += 1;
		if( _index > _tokens.size() )
		{
			throw new std::string{"What?"};
		}
	}

	bool has_tokens() { return _index < _tokens.size(); }
};

std::unique_ptr<FunctionAST> parse_function(TokenCursor& cursor);
std::unique_ptr<ExprAST> parse_expression(TokenCursor& cursor);

std::unique_ptr<NumberExprAST>
parse_number_expression(TokenCursor& cursor)
{}

std::unique_ptr<FunctionAST>
parse_module_top_level_item(TokenCursor& cursor)
{
	switch( cursor.peek().type )
	{
	case TokenType::fn:
	{
		cursor.adv();
		return parse_function(cursor);
	}
	default:
		std::cout << "Expected function definition" << std::endl;
		break;
	}

	return nullptr;
}

std::unique_ptr<FunctionAST>
parse_module(TokenCursor& cursor)
{
	return parse_module_top_level_item(cursor);
}

std::unique_ptr<ExprAST>
parse_block(TokenCursor& cursor)
{
	std::vector<std::unique_ptr<ExprAST>> stmts;

	auto tok = cursor.peek();
	if( tok.type != TokenType::open_curly )
	{
		std::cout << "Expected '{'" << std::endl;
	}

	cursor.adv();
	tok = cursor.peek();
	while( tok.type != TokenType::close_curly )
	{
		switch( tok.type )
		{
		case TokenType::literal:
			stmts.push_back(std::move(std::move(parse_expression(cursor))));
			break;
		case TokenType::return_keyword:
			cursor.adv();
			stmts.push_back(
				std::move(std::make_unique<ReturnExprAST>(std::move(parse_expression(cursor)))));
			break;
		default:
			std::cout << "Expected expression or return statement" << std::endl;
			break;
		}

		tok = cursor.peek();
		if( tok.type != TokenType::semicolon )
		{
			std::cout << "Expected ';'" << std::endl;
			return nullptr;
		}

		cursor.adv();
		tok = cursor.peek();
	}

	return std::make_unique<BlockAST>(std::move(stmts));
}

int
get_token_precedence(Token const& token)
{
	// Make sure it's a declared binop.
	int TokPrec = BinopPrecedence[*token.start];
	if( TokPrec <= 0 )
		return -1;
	return TokPrec;
}

std::unique_ptr<ExprAST>
parse_bin_op(TokenCursor& cursor, int ExprPrec, std::unique_ptr<ExprAST> LHS)
{
	// If this is a binop, find its precedence.
	while( true )
	{
		auto cur = cursor.peek();
		int TokPrec = get_token_precedence(cur);

		// If this is a binop that binds at least as tightly as the current binop,
		// consume it, otherwise we are done.
		if( TokPrec < ExprPrec )
			return LHS;

		// This is a binary operation because TokPrec would be less than ExprPrec if
		// the next token was not a bin op (e.g. if statement or so.)
		char BinOp = *cursor.peek().start;
		cursor.adv();

		// Parse the primary expression after the binary operator.
		auto RHS = parse_expression(cursor);
		if( !RHS )
			return nullptr;

		// If BinOp binds less tightly with RHS than the operator after RHS, let
		// the pending operator take RHS as its LHS.
		int NextPrec = get_token_precedence(cursor.peek());
		if( TokPrec < NextPrec )
		{
			RHS = parse_bin_op(cursor, TokPrec + 1, std::move(RHS));
			if( !RHS )
				return nullptr;
		}

		// Merge LHS/RHS.
		LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
	}
}

std::unique_ptr<NumberExprAST>
parse_expression_value(TokenCursor& cursor)
{
	auto tok = cursor.peek();
	if( tok.type != TokenType::literal )
	{
		std::cout << "Expected literal" << std::endl;
		return nullptr;
	}
	auto sz = std::string{tok.start, tok.size};
	int val = std::stoi(sz);
	cursor.adv();
	return std::make_unique<NumberExprAST>(val);
}

std::unique_ptr<ExprAST>
parse_expression(TokenCursor& cursor)
{
	auto LHS = parse_expression_value(cursor);
	if( !LHS )
	{
		return nullptr;
	}

	auto OP = parse_bin_op(cursor, 0, std::move(LHS));

	return OP;
}

std::unique_ptr<PrototypeAST>
parse_function_proto(TokenCursor& cursor)
{
	Token tok_fn_name = cursor.peek();
	if( tok_fn_name.type != TokenType::identifier )
	{
		std::cout << "Expected function name identifier" << std::endl;
		return nullptr;
	}

	cursor.adv();
	if( cursor.peek().type != TokenType::open_paren )
	{
		std::cout << "Expected '('" << std::endl;
		return nullptr;
	}

	// TODO: Identifiers for args
	cursor.adv();
	if( cursor.peek().type != TokenType::close_paren )
	{
		std::cout << "Expected ')'" << std::endl;
		return nullptr;
	}

	cursor.adv();
	if( cursor.peek().type != TokenType::colon )
	{
		std::cout << "Expected ':'" << std::endl;
		return nullptr;
	}

	cursor.adv();
	Token tok_fn_return_type = cursor.peek();
	if( tok_fn_return_type.type != TokenType::identifier )
	{
		std::cout << "Expected return type" << std::endl;
		return nullptr;
	}

	cursor.adv();
	return std::make_unique<PrototypeAST>(
		std::string{tok_fn_name.start, tok_fn_name.size}, std::vector<std::string>{});
}

std::unique_ptr<ExprAST>
parse_function_body(TokenCursor& cursor)
{
	return parse_block(cursor);
}

std::unique_ptr<FunctionAST>
parse_function(TokenCursor& cursor)
{
	auto proto = parse_function_proto(cursor);
	if( !proto )
	{
		return nullptr;
	}

	// TODO: Pass in proto to check return type
	auto definition = parse_function_body(cursor);
	if( !definition )
	{
		return nullptr;
	}

	return std::make_unique<FunctionAST>(std::move(proto), std::move(definition));
}

void
codegen(std::vector<Token> const& tokens)
{
	std::vector<ExprAST*> asts;
	asts.reserve(30);
	TokenCursor cursor{tokens};
	auto mod = parse_module(cursor);
	if( !mod )
	{
		return;
	}
	mod->codegen();
}

#include <iostream>
int
main()
{
	TheContext = std::make_unique<LLVMContext>();
	TheModule = std::make_unique<Module>("this_module", *TheContext);
	// Create a new builder for the module.
	Builder = std::make_unique<IRBuilder<>>(*TheContext);

	BinopPrecedence['<'] = 10;
	BinopPrecedence['+'] = 20;
	BinopPrecedence['-'] = 20;
	BinopPrecedence['*'] = 40; // highest.

	char const buf[] = "fn func(): i8 { return 9*8; }";
	Lexer lex{buf};

	auto tokens = lex.lex();

	Lexer::print_tokens(tokens);

	codegen(tokens);

	// IRBuilder<> Builder{TheContext};
	// Module Mod{"main", TheContext};
	// FunctionType* FT = FunctionType::get(Type::getDoubleTy(TheContext), false);
	// Function* Func = Function::Create(FT, Function::ExternalLinkage, "floater", Mod);

	// BasicBlock* Block = BasicBlock::Create(TheContext, "entry", Func);
	// Builder.SetInsertPoint(Block);

	// auto const L = ConstantFP::get(TheContext, APFloat(4.5));
	// auto const R = ConstantFP::get(TheContext, APFloat(5.9));

	// Value* ResVar = Builder.CreateFAdd(L, R, "addtmp");

	// Builder.CreateRet(ResVar);

	// verifyFunction(*Func);

	std::string Str;
	raw_string_ostream OS(Str);

	TheModule->print(OS, nullptr);

	std::cout << Str;

	// auto CPU = "generic";
	// auto Features = "";

	// InitializeNativeTarget();
	// InitializeNativeTargetAsmParser();
	// InitializeNativeTargetAsmPrinter();
	// TargetOptions opt;
	// auto RM = Optional<Reloc::Model>();

	// // auto TargetTriple = sys::getDefaultTargetTriple();
	// std::string TargetTriple = "aarch64-app-darwin";
	// std::cout << "Target: " << TargetTriple << std::endl;

	// std::string Error;
	// auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);
	// if( !Target )
	// {
	// 	errs() << Error;
	// 	return 1;
	// }

	// auto TheTargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

	// Mod.setDataLayout(TheTargetMachine->createDataLayout());
	// Mod.setTargetTriple(TargetTriple);

	// auto Filename = "output.o";
	// std::error_code EC;
	// raw_fd_ostream dest(Filename, EC, sys::fs::OF_None);

	// if( EC )
	// {
	// 	errs() << "Could not open file: " << EC.message();
	// 	return 1;
	// }

	// legacy::PassManager pass;
	// auto FileType = CGFT_ObjectFile;

	// if( TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType) )
	// {
	// 	errs() << "TheTargetMachine can't emit a file of this type";
	// 	return 1;
	// }

	// pass.run(Mod);
	// dest.flush();

	// outs() << "Wrote " << Filename << "\n";

	return 0;
}