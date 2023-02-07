#include "Codegen.h"

#include "Codegen/CGNotImpl.h"
#include "Codegen/codegen_function.h"
#include "Codegen/lookup.h"
#include "ast2/AstCasts.h"

using namespace cg;
using namespace ast;

static void
establish_llvm_builtin_types(
	CG& cg, sema::Types& types, std::map<sema::Type const*, llvm::Type*>& lut)
{
	lut.emplace(types.u8_type(), llvm::Type::getInt8Ty(*cg.Context));
	lut.emplace(types.u16_type(), llvm::Type::getInt16Ty(*cg.Context));
	lut.emplace(types.u32_type(), llvm::Type::getInt32Ty(*cg.Context));
	lut.emplace(types.i8_type(), llvm::Type::getInt8Ty(*cg.Context));
	lut.emplace(types.i16_type(), llvm::Type::getInt16Ty(*cg.Context));
	lut.emplace(types.i32_type(), llvm::Type::getInt32Ty(*cg.Context));
	lut.emplace(types.void_type(), llvm::Type::getVoidTy(*cg.Context));
}

static llvm::Value*
promote_to_value(CG& cg, llvm::Value* Val)
{
	// TODO: Opaque pointers, don't rel
	auto PointedToType = Val->getType()->getPointerElementType();
	return cg.Builder->CreateLoad(PointedToType, Val);
}

static llvm::Value*
__deprecate_promote_to_value(CG& cg, llvm::Value* Val)
{
	// TODO: Opaque pointers, don't rel
	if( Val->getType()->isPointerTy() )
		return promote_to_value(cg, Val);
	else
		return Val;
}

CG::CG(sema::Sema2& sema)
	: sema(sema)
{
	Context = std::make_unique<llvm::LLVMContext>();
	Module = std::make_unique<llvm::Module>("this_module", *Context);
	// Create a new builder for the module.
	Builder = std::make_unique<llvm::IRBuilder<>>(*Context);
	// TODO: Populate builtin types that automatically map the llvm types.

	establish_llvm_builtin_types(*this, sema.types, this->types);
}

CGResult<CGExpr>
CG::codegen_module(ir::IRModule* mod)
{
	for( auto tls : *mod->stmts )
	{
		auto tlsr = codegen_tls(tls);
		if( !tlsr.ok() )
			return tlsr;
	}

	return CGExpr();
}

CGResult<CGExpr>
CG::codegen_tls(ir::IRTopLevelStmt* tls)
{
	switch( tls->type )
	{
	case ir::IRTopLevelType::ExternFn:
		return codegen_extern_fn(tls->stmt.extern_fn);
	case ir::IRTopLevelType::Function:
		return codegen_function(*this, tls->stmt.fn);
	case ir::IRTopLevelType::Struct:
		return codegen_struct(tls->stmt.struct_decl);
	}

	return NotImpl();
}

CGResult<CGExpr>
CG::codegen_stmt(ir::IRStmt* stmt)
{
	switch( stmt->type )
	{
	case ir::IRStmtType::ExprStmt:
		return codegen_expr(stmt->stmt.expr);
	case ir::IRStmtType::Return:
		return codegen_return(stmt->stmt.ret);
	case ir::IRStmtType::Assign:
		return codegen_assign(stmt->stmt.assign);
	case ir::IRStmtType::Let:
		return codegen_let(stmt->stmt.let);
	}

	return NotImpl();
}

CGResult<CGExpr>
CG::codegen_expr(ir::IRExpr* expr)
{
	return codegen_expr(expr, std::optional<CGExpr>());
}

CGResult<CGExpr>
CG::codegen_expr(ir::IRExpr* expr, std::optional<CGExpr> lvalue)
{
	switch( expr->type )
	{
	case ir::IRExprType::Call:
		return codegen_call(expr->expr.call, lvalue);
	case ir::IRExprType::Id:
		return codegen_id(expr->expr.id);
	case ir::IRExprType::NumberLiteral:
		return codegen_number_literal(expr->expr.num_literal);
	case ir::IRExprType::StringLiteral:
		return codegen_string_literal(expr->expr.str_literal);
	case ir::IRExprType::ValueDecl:
		return codegen_value_decl(expr->expr.decl);
	case ir::IRExprType::BinOp:
		return codegen_binop(expr->expr.binop);
	case ir::IRExprType::MemberAccess:
		return codegen_member_access(expr->expr.member_access);
	}

	return NotImpl();
}

CGResult<CGExpr>
CG::codegen_extern_fn(ir::IRExternFn* extern_fn)
{
	return codegen_function_proto(*this, extern_fn->proto);
}

CGResult<CGExpr>
CG::codegen_return(ir::IRReturn* ret)
{
	auto maybe_fn_ctx = current_function;
	assert(maybe_fn_ctx.has_value());

	auto fn_ctx = current_function.value();

	auto exprr = codegen_expr(ret->expr);
	if( !exprr.ok() )
		return exprr;
	auto expr = exprr.unwrap();

	auto rt = fn_ctx.fn_type->get_return_type().value();
	if( sema.types.equal_types(rt, sema.types.VoidType()) )
	{
		Builder->CreateRetVoid();
	}
	else if( rt.type->is_struct_type() && rt.indirection_level == 0 )
	{
		auto Function = fn_ctx.Fn;
		auto MaybeSRet = Function->getArg(0);
		if( MaybeSRet->hasAttribute(llvm::Attribute::StructRet) )
		{
			auto SRet = Function->getArg(0);
			auto Expr = expr.as_value();

			// TODO: Compute alignment from member
			auto Size =
				Module->getDataLayout().getTypeAllocSize(Expr->getType()->getPointerElementType());
			auto Align =
				Module->getDataLayout().getPrefTypeAlign(Expr->getType()->getPointerElementType());

			Builder->CreateMemCpy(SRet, Align, Expr, Align, Size);
			Builder->CreateRetVoid();
		}
		else
		{
			assert(0); // ???
			Builder->CreateRetVoid();
		}
	}
	else
	{
		Builder->CreateRet(expr.as_value());
	}

	return CGExpr();
}

CGResult<CGExpr>
CG::codegen_member_access(ir::IRMemberAccess* ma)
{
	auto exprr = codegen_expr(ma->expr);
	if( !exprr.ok() )
		return exprr;
	auto expr = exprr.unwrap();
	auto ExprValue = expr.as_value();

	auto expr_ty = ma->expr->type_instance;
	assert(expr_ty.type->is_struct_type() && expr_ty.indirection_level == 0);
	auto llvm_expr_tyr = get_type(*this, expr_ty);
	if( !llvm_expr_tyr.ok() )
		return llvm_expr_tyr;
	auto ExprTy = llvm_expr_tyr.unwrap();

	auto struct_ty_name = expr_ty.type->get_name();

	auto member_name = *ma->member_name;
	auto maybe_member = expr_ty.type->get_member(member_name);
	assert(maybe_member.has_value());

	auto member = maybe_member.value();
	auto llvm_member_tyr = get_type(*this, member.type);
	if( !llvm_member_tyr.ok() )
		return llvm_member_tyr;
	auto MemberTy = llvm_member_tyr.unwrap();

	auto MemberPtr =
		Builder->CreateStructGEP(ExprTy, ExprValue, member.idx, struct_ty_name + "." + member_name);

	return CGExpr(MemberPtr);
}

CGResult<CGExpr>
CG::codegen_let(ir::IRLet* let)
{
	auto name = let->name;
	auto type = let->assign->rhs->type_instance;
	auto typer = get_type(*this, type);
	if( !typer.ok() )
		return typer;
	auto Type = typer.unwrap();

	llvm::AllocaInst* Alloca = Builder->CreateAlloca(Type, nullptr, *name);
	values.emplace(*name, Alloca);

	auto assignr = codegen_assign(let->assign);
	if( !assignr.ok() )
		return assignr;

	return CGExpr(Alloca);
}

// CGResult<CGExpr>
// CG::codegen_assign(ir::IRAssign* assign)
// {
// 	return codegen_assign(assign, std::optional<CGExpr>());
// }

CGResult<CGExpr>
CG::codegen_assign(ir::IRAssign* assign)
{
	auto lhsr = codegen_expr(assign->lhs);
	if( !lhsr.ok() )
		return lhsr;

	auto lexpr = lhsr.unwrap();
	auto rhsr = codegen_expr(assign->rhs, lexpr);
	if( !rhsr.ok() )
		return rhsr;

	auto rexpr = rhsr.unwrap();

	// TODO: The RHS might be a Struct type name.
	// E.g.
	//
	// let my_point = Point;
	//
	// In this case, assignment is a no op.
	// TODO: Later, insert a constructor call?
	if( rexpr.type == CGExprType::Empty )
		return CGExpr();

	// TODO: lvalue rvalue?
	auto LValue = lexpr.as_value();
	auto RValue = rexpr.as_value();

	assert(LValue && RValue && "nullptr for assignment!");

	// TODO: Promote to value really only needs to be done for allocas??
	// Confusing, we'll see where this goes.
	switch( assign->op )
	{
	case ast::AssignOp::add:
	{
		auto LValuePromoted = promote_to_value(*this, LValue);
		auto TempRValue = Builder->CreateAdd(RValue, LValuePromoted);
		Builder->CreateStore(TempRValue, LValue);
	}
	break;
	case ast::AssignOp::sub:
	{
		auto LValuePromoted = promote_to_value(*this, LValue);
		auto TempRValue = Builder->CreateSub(LValuePromoted, RValue);
		Builder->CreateStore(TempRValue, LValue);
	}
	break;
	case ast::AssignOp::mul:
	{
		auto LValuePromoted = promote_to_value(*this, LValue);
		auto TempRValue = Builder->CreateMul(RValue, LValuePromoted);
		Builder->CreateStore(TempRValue, LValue);
	}
	break;
	case ast::AssignOp::div:
	{
		auto LValuePromoted = promote_to_value(*this, LValue);
		auto TempRValue = Builder->CreateSDiv(RValue, LValuePromoted);
		Builder->CreateStore(TempRValue, LValue);
	}
	break;
	case ast::AssignOp::assign:
		// TODO: Constant expressions return their exact values,
		// LValue expressions return a pointer to them, so LValues need
		// to be promoted. Need to create LValue type
		auto RValuePromoted = __deprecate_promote_to_value(*this, RValue);
		Builder->CreateStore(RValuePromoted, LValue);
		break;
	}

	return lhsr.unwrap();
}

CGResult<CGExpr>
CG::codegen_number_literal(ir::IRNumberLiteral* lit)
{
	//
	llvm::Value* ConstInt = llvm::ConstantInt::get(*Context, llvm::APInt(32, lit->val, true));

	return CGExpr(ConstInt, true);
}

CGResult<CGExpr>
CG::codegen_string_literal(ir::IRStringLiteral* lit)
{
	//
	auto Literal = llvm::ConstantDataArray::getString(*Context, lit->value->c_str(), true);

	llvm::GlobalVariable* GVStr = new llvm::GlobalVariable(
		*Module, Literal->getType(), true, llvm::GlobalValue::InternalLinkage, Literal);
	llvm::Constant* zero = llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(*Context));
	llvm::Constant* indices[] = {zero, zero};
	llvm::Constant* StrVal =
		llvm::ConstantExpr::getGetElementPtr(Literal->getType(), GVStr, indices);

	return CGExpr(StrVal, true);
}

CGResult<CGExpr>
CG::codegen_value_decl(ir::IRValueDecl* decl)
{
	auto valuer = get_value(*this, *decl->name);
	assert(valuer.has_value());

	return valuer.value();
}

CGResult<CGExpr>
CG::codegen_binop(ir::IRBinOp* binop)
{
	auto lhsr = codegen_expr(binop->lhs);
	if( !lhsr.ok() )
		return lhsr;

	auto rhsr = codegen_expr(binop->rhs);
	if( !rhsr.ok() )
		return rhsr;

	auto lexpr = lhsr.unwrap();
	auto rexpr = rhsr.unwrap();
	// TODO: lvalue rvalue?
	auto LValue = lexpr.as_value();
	auto RValue = rexpr.as_value();

	assert(LValue && RValue && "nullptr for assignment!");

	// TODO: If the values are imediate then we don't need to promote.
	// TODO: If both values are constant, just do the computation.
	auto L = __deprecate_promote_to_value(*this, LValue); // promote_to_value(*this, LValue);
	auto R = __deprecate_promote_to_value(*this, RValue); // promote_to_value(*this, RValue);

	assert(L->getType()->isIntegerTy() && R->getType()->isIntegerTy());

	auto Op = binop->op;
	switch( Op )
	{
	case BinOp::plus:
		return CGExpr(Builder->CreateAdd(L, R, "addtmp"));
	case BinOp::minus:
		return CGExpr(Builder->CreateSub(L, R, "subtmp"));
	case BinOp::star:
		return CGExpr(Builder->CreateMul(L, R, "multmp"));
	case BinOp::slash:
		return CGExpr(Builder->CreateSDiv(L, R, "divtmp"));
	case BinOp::gt:
		return CGExpr(Builder->CreateICmpUGT(L, R, "cmptmp"));
	case BinOp::gte:
		return CGExpr(Builder->CreateICmpUGE(L, R, "cmptmp"));
	case BinOp::lt:
		return CGExpr(Builder->CreateICmpULT(L, R, "cmptmp"));
	case BinOp::lte:
		return CGExpr(Builder->CreateICmpULE(L, R, "cmptmp"));
	case BinOp::cmp:
		return CGExpr(Builder->CreateICmpEQ(L, R, "cmptmp"));
	case BinOp::ne:
		return CGExpr(Builder->CreateICmpNE(L, R, "cmptmp"));
	case BinOp::and_lex:
		return CGExpr(Builder->CreateAnd(L, R, "cmptmp"));
	case BinOp::or_lex:
		return CGExpr(Builder->CreateOr(L, R, "cmptmp"));
	default:
		return NotImpl();
	}
}

CGResult<CGExpr>
CG::codegen_call(ir::IRCall* call, std::optional<CGExpr> lvalue)
{
	auto call_target_type = call->call_target->type_instance;
	assert(call_target_type.type->is_function_type() && call_target_type.indirection_level <= 1);

	auto exprr = codegen_expr(call->call_target);
	if( !exprr.ok() )
		return exprr;

	auto expr = exprr.unwrap();
	if( expr.type != CGExprType::FunctionValue )
		return CGError("Call target is not a value??"); // Assert here?

	auto Function = expr.data.fn;
	auto return_type = call_target_type.type->get_return_type().value();

	// Assert value is correct?
	// Semantic analysis should've guaranteed this is a function.
	// if( !Value->getType()->isPointerTy() &&
	// 	!Value->getType()->getPointerElementType()->isFunctionTy() )
	// {
	// 	std::cout << "Expected function type" << std::endl;
	// 	return;
	// }

	// llvm::Function* Function = static_cast<llvm::Function*>(Value);

	std::vector<llvm::Value*> ArgsV;
	if( Function->getArg(0)->hasAttribute(llvm::Attribute::StructRet) )
	{
		// Call with sret
		auto rtr = get_type(*this, return_type);
		if( !rtr.ok() )
			return rtr;
		auto SRetType = rtr.unwrap();

		if( lvalue.has_value() )
		{
			ArgsV.push_back(lvalue.value().as_value());
		}
		else
		{
			llvm::AllocaInst* SRetAlloca = Builder->CreateAlloca(SRetType, nullptr, "Wow");
			ArgsV.push_back(SRetAlloca);
		}

		return_type = sema::TypeInstance::OfType(sema.types.void_type());
	}

	for( auto arg_expr_node : *call->args->args )
	{
		auto arg_exprr = codegen_expr(arg_expr_node);
		if( !arg_exprr.ok() )
			return arg_exprr;

		auto arg_expr = arg_exprr.unwrap();
		if( arg_expr.type != CGExprType::Value )
			return CGError("Arg is not a value??"); // Assert here?

		auto ArgValue = arg_expr.data.value;

		// TODO: Again, constants return as values, and allocas are pointers.
		// Need to consolidate this.
		auto ArgValuePromoted =
			arg_expr.literal ? ArgValue : __deprecate_promote_to_value(*this, ArgValue);
		ArgsV.push_back(ArgValuePromoted);
	}

	// https://github.com/ark-lang/ark/issues/362

	if( sema.types.equal_types(return_type, sema.types.VoidType()) )
	{
		auto CallValue = Builder->CreateCall(Function, ArgsV);
		if( Function->getArg(0)->hasAttribute(llvm::Attribute::StructRet) )
			return CGExpr();
		else
			return CGExpr();
	}
	else
	{
		auto CallValue = Builder->CreateCall(Function, ArgsV, "call");
		return CGExpr(CallValue);
	}
}

CGResult<CGExpr>
CG::codegen_id(ir::IRId* id)
{
	// auto iter_type = types.find(id->type_instance.type);
	auto value = get_value(*this, *id->name);
	if( value.has_value() )
		return value.value();

	// TODO: This supports 'let my_point = Point' initialization.
	// I can see the "Codegen Id" function getting a little wild.
	// Need to rethink it.
	// TODO: This should be get type by name...
	// Otherwise bad codegen.
	auto maybe_type = get_type(*this, id->type_instance);
	if( maybe_type.ok() )
		return CGExpr();

	return CGError("Undeclared identifier! " + *id->name);
}

CGResult<CGExpr>
CG::codegen_struct(ir::IRStruct* st)
{
	Vec<llvm::Type*> members;
	for( auto& member : *st->members )
	{
		auto value_decl = member.second;
		auto typerr = get_type(*this, value_decl->type_decl);
		if( !typerr.ok() )
			return typerr;

		members.push_back(typerr.unwrap());
	}

	auto struct_type = st->struct_type;
	auto name = struct_type->get_name();
	llvm::StructType* StructTy = llvm::StructType::create(*Context, members, name);

	this->types.emplace(struct_type, StructTy);

	return CGExpr();
}

std::optional<llvm::Type*>
CG::find_type(sema::Type const* ty)
{
	auto t = types.find(ty);
	if( t != types.end() )
		return t->second;
	else
		return std::optional<llvm::Type*>();
}

// template<typename T>
// CGResult<T>
// expected(ast::AstNode* node, Cast<T> (*cast)(ast::AstNode* node))
// {
// 	auto castr = cast(node);
// 	if( !castr.ok() )
// 		return CGError("Expected type '" + ast::to_string(node->type) + "'");

// 	return castr.unwrap();
// }

// CGResult<CGExpr>
// CG::codegen(ast::AstNode* node)
// {
// 	switch( node->type )
// 	{
// 	case NodeType::Invalid:
// 		return CGError("Invalid NodeType in Codegen.");
// 	case NodeType::Module:
// 		return codegen_module(node);
// 	case NodeType::Fn:
// 		return codegen_fn(node);
// 	case NodeType::FnProto:
// 		break;
// 	case NodeType::FnParamList:
// 		break;
// 	case NodeType::ValueDecl:
// 		break;
// 	case NodeType::FnCall:
// 		return codegen_fn_call(node);
// 	case NodeType::ExprList:
// 		break;
// 	case NodeType::Block:
// 		break;
// 	case NodeType::BinOp:
// 		break;
// 	case NodeType::Id:
// 		return codegen_id(node);
// 	case NodeType::Assign:
// 		break;
// 	case NodeType::If:
// 		break;
// 	case NodeType::Let:
// 		break;
// 	case NodeType::Return:
// 		return codegen_fn_return(node);
// 	case NodeType::Struct:
// 		break;
// 	case NodeType::MemberDef:
// 		break;
// 	case NodeType::While:
// 		break;
// 	case NodeType::For:
// 		break;
// 	case NodeType::StringLiteral:
// 		break;
// 	case NodeType::NumberLiteral:
// 		return codegen_number_literal(node);
// 	case NodeType::TypeDeclarator:
// 		break;
// 	case NodeType::MemberAccess:
// 		break;
// 	case NodeType::Expr:
// 		return codegen_expr(node);
// 	case NodeType::Stmt:
// 		return codegen_stmt(node);
// 	}

// 	return CGError("Unhandled ast NodeType in Codegen.");
// }

// CGResult<CGExpr>
// CG::codegen_module(ast::AstNode* node)
// {
// 	auto result = expected(node, NodeType::Module);
// 	if( !result.ok() )
// 		return result;

// 	auto mod = node->data.mod;

// 	for( auto statement : mod.statements )
// 	{
// 		auto statement_result = codegen(statement);
// 		// Don't really care about the sema result here.
// 		if( !statement_result.ok() )
// 			return statement_result;
// 	}

// 	return CGExpr{};
// }

// CGResult<CGExpr>
// CG::codegen_fn(ast::AstNode* node)
// {
// 	auto result = expected(node, NodeType::Fn);
// 	if( !result.ok() )
// 		return result;

// 	auto scope = get_scope(node);

// 	for( auto statement : mod.statements )
// 	{
// 		auto statement_result = codegen(statement);
// 		// Don't really care about the sema result here.
// 		if( !statement_result.ok() )
// 			return statement_result;
// 	}

// 	return CGExpr{};
// }

// CGResult<llvm::Type*>
// codegen_fn_param(CG& cg, ast::AstNode* node)
// {
// 	auto value_declr = ::expected(node, ast::as_value_decl);
// 	if( !value_declr.ok() )
// 		return value_declr;

// 	auto type_declr = ::expected(node, ast::as_type_decl);
// 	if( !type_declr.ok() )
// 		return type_declr;

// 	auto type_decl = type_declr.unwrap();
// 	auto type_name = type_decl.name;

// 	auto scope = cg.get_scope(node);
// 	// scope
// }

// CGResult<std::vector<llvm::Type*>>
// codegen_fn_params(ast::AstNode* node)
// {
// 	auto paramsr = ::expected(node, ast::as_fn_param_list);
// 	if( !paramsr.ok() )
// 		return paramsr;

// 	auto params = paramsr.unwrap();

// 	std::vector<llvm::Type*> ParamTys;
// 	for( auto param : params.params )
// 	{
// 		auto paramr = codegen_fn_param(param);
// 		if( !paramr.ok() )
// 			return paramr;

// 		ParamTys.push_back(ty);
// 	}

// 	return ParamTys;
// }

// CGResult<CGExpr>
// CG::codegen_fn_proto(ast::AstNode* node)
// {
// 	auto protor = ::expected(node, ast::as_fn_proto);
// 	if( !protor.ok() )
// 		return CGExpr{};

// 	auto scope = get_scope(node);

// 	auto proto = protor.unwrap();
// 	auto namesr = as_id(proto.name);
// 	if( !namesr.ok() )
// 		return namesr;

// 	auto name_node = namesr.unwrap();
// 	auto name = name_node.name;

// 	auto paramsr = codegen_fn_params(proto.params);
// 	if( !paramsr )
// 		return paramsr;

// 	auto ParamsTys = paramsr.unwrap();

// 	llvm::FunctionType* FT = FunctionType::get(ret_ty, ParamsTys, false);

// 	llvm::Function* Function =
// 		llvm::Function::Create(FT, llvm::Function::ExternalLinkage, name, Module.get());
// 	scope->Function = Function;

// 	Functions.emplace(name, v)
// }

// CGResult<CGExpr>
// CG::codegen_id(ast::AstNode* node)
// {
// 	//
// }

// CGResult<CGExpr>
// CG::codegen_block(ast::AstNode* node)
// {
// 	auto blockr = ::expected(node, ast::as_block);
// 	if( !blockr.ok() )
// 		return CGExpr{};

// 	auto block = blockr.unwrap();

// 	for( auto stmt : block.statements )
// 	{
// 		auto stmtr = codegen(stmt);
// 		if( !stmtr.ok() )
// 			return stmtr;
// 	}

// 	return CGExpr{};
// }

// CGResult<CGExpr>
// CG::codegen_fn_call(ast::AstNode* node)
// {
// 	auto fn_callr = ::expected(node, ast::as_fn_call);
// 	if( !fn_callr.ok() )
// 		return CGExpr{};

// 	auto fn_call = fn_callr.unwrap();
// 	auto exprr = codegen_expr(fn_call.call_target);
// 	if( !exprr.ok() )
// 		return CGExpr{};

// 	const CallExprVal = exprr.unwrap();
// }

// CGResult<CGExpr>
// CG::codegen_fn_return(ast::AstNode* node)
// {
// 	auto fn_returnr = ::expected(node, ast::as_fn_return);
// 	if( !fn_returnr.ok() )
// 		return CGExpr{};
// }

// CGResult<CGExpr>
// CG::codegen_number_literal(ast::AstNode* node)
// {
// 	auto number_litr = ::expected(node, ast::as_number_literal);
// 	if( !number_litr.ok() )
// 		return CGExpr{};
// }

// CGResult<CGExpr>
// CG::codegen_expr(ast::AstNode* node)
// {
// 	auto exprr = ::expected(node, ast::as_expr);
// 	if( !exprr.ok() )
// 		return CGExpr{};
// }

// CGResult<CGExpr>
// CG::codegen_stmt(ast::AstNode* node)
// {
// 	auto stmtr = ::expected(node, ast::as_stmt);
// 	if( !stmtr.ok() )
// 		return CGExpr{};
// }

// CGScope*
// CG::get_scope(ast::AstNode* node)
// {
// 	auto sematag = ast.query<sema::Sema2>(node);

// 	auto s = sematag->scope;

// 	auto iter_scopes = scopes.find(s);
// 	if( iter_scopes != scopes.end() )
// 	{
// 		return &iter_scopes->second;
// 	}
// 	else
// 	{
// 		return nullptr;
// 	}
// }

// sema::Scope*
// CG::get_sema_scope(ast::AstNode* node)
// {
// 	auto sematag = ast.query<sema::Sema2>(node);
// 	return sematag->scope;
// }