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
	{
		// auto ex = sema_fn(sema, ast);
		// if( !ex.ok() )
		// return ex;

		// return sema.TLS(ex.unwrap());
		return SemaError("Not Impl.");
	}
	default:
		return SemaError("Unsupported NodeType as TLS.");
	}
}