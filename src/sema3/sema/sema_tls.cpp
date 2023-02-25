#include "sema_tls.h"

#include "../sema_expected.h"
#include "sema_fn.h"

using namespace sema;

SemaResult<ir::ActionResult>
sema::sema_tls(Sema& sema, ast::AstNode* ast)
{
	//
	switch( ast->type )
	{
	case ast::NodeType::Fn:
		return sema_fn(sema, ast);
	case ast::NodeType::ExternFn:
		return sema_extern_fn(sema, ast);
	default:
		return SemaError("Unsupported NodeType as TLS.");
	}
}