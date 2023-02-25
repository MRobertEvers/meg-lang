#include "sema_return.h"

#include "../sema_expected.h"
#include "ast2/AstCasts.h"

using namespace sema;

SemaResult<ir::ActionResult>
sema::sema_return(Sema& sema, ast::AstNode* ast)
{
	auto returnr = expected(ast, ast::as_fn_return);
	if( !returnr.ok() )
		return returnr;
	auto return_expr = returnr.unwrap();

	sema.builder().create_return(nullptr);

	return ir::ActionResult();

	// auto retr = sema_expr(sema, return_expr.expr);
	// if( !retr.ok() )
	// 	return retr;
	// auto ret = retr.unwrap();

	// auto maybe_expected_type = sema.get_expected_return();
	// if( !maybe_expected_type.has_value() )
	// 	return SemaError("Return statement outside function?");

	// auto expected_type = maybe_expected_type.value();
	// if( !sema.types.equal_types(expected_type, ret->type_instance) )
	// 	return SemaError("Incorrect return type.");

	// return sema.Return(ast, retr.unwrap());
}