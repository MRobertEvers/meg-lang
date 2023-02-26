#include "sema_call.h"

#include "../sema_expected.h"
#include "ast2/AstCasts.h"
#include "sema_expr.h"

using namespace sema;

static SemaResult<std::vector<ir::Inst*>>
sema_fn_args(Sema& sema, ast::AstNode* ast, ir::Type const& fn_type)
{
	std::vector<ir::Inst*> instructions;
	auto ast_expr_list = expected(ast, ast::as_expr_list);
	if( !ast_expr_list.ok() )
		return ast_expr_list;
	auto args_node = ast_expr_list.unwrap();

	int arg_count = 0;
	for( auto ast_expr : args_node.exprs )
	{
		if( arg_count >= fn_type.get_member_count() && !fn_type.is_var_arg() )
			return SemaError("Too many arguments!");

		auto expr_result = sema_expr(sema, ast_expr);
		if( !expr_result.ok() )
			return expr_result;

		auto expr = expr_result.unwrap();

		auto expr_type = expr.action().type;

		if( arg_count < fn_type.get_member_count() )
		{
			if( !sema.types().equal_types(expr_type, fn_type.get_member(arg_count).type) )
				return SemaError("Mismatched argument type.");
		}

		instructions.push_back(expr.action().inst);

		arg_count += 1;
	}

	if( fn_type.is_var_arg() )
	{
		if( arg_count < fn_type.get_member_count() - 1 )
			return SemaError("Missing arguments.");
	}
	else
	{
		if( arg_count != fn_type.get_member_count() )
			return SemaError("Missing arguments.");
	}

	return instructions;
}

SemaResult<ir::ActionResult>
sema::sema_call(Sema& sema, ast::AstNode* ast)
{
	auto ast_call = expected(ast, ast::as_fn_call);
	if( !ast_call.ok() )
		return ast_call;
	auto call_node = ast_call.unwrap();

	auto call_target_result = sema_expr(sema, call_node.call_target);
	if( !call_target_result.ok() )
		return call_target_result;
	auto call_target = call_target_result.unwrap().action();

	auto call_target_type = call_target.type;
	if( !call_target_type.is_function_type() )
		return SemaError("...is not a function!");

	auto args_result = sema_fn_args(sema, call_node.args, *call_target_type.type);
	if( !args_result.ok() )
		return args_result;

	sema.builder().create_call(call_target.inst, args_result.unwrap(), call_target.type);

	return ir::ActionResult();
}