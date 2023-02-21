#pragma once
#include "../IR.h"
#include "../Sema2.h"
#include "../SemaResult.h"
#include "ast2/Ast.h"
#include "ast2/AstCasts.h"

namespace sema
{
struct sema_id_t
{
	union
	{
		ir::IRId* id;
		ir::IRInitializer* initializer;
	};
	enum class Type
	{
		Id,
		Initializer
	} type;

	sema_id_t(ir::IRId* id)
		: id(id)
		, type(sema_id_t::Type::Id)
	{}
	sema_id_t(ir::IRInitializer* id)
		: initializer(id)
		, type(sema_id_t::Type::Initializer)
	{}
};
SemaResult<sema_id_t> sema_id(Sema2& sema, ast::AstNode* ast);
} // namespace sema