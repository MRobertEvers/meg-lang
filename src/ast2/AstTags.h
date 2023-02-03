#pragma once
#include "AstNode.h"

#include <map>
#include <memory>

namespace ast
{
class IAstTag
{
public:
	virtual ~IAstTag(){};
};

class AstTags
{
private:
	// Lookup node -> Tag Type -> Tag
	std::map<AstNode*, std::map<int, std::unique_ptr<IAstTag>>> lut;

public:
	template<
		typename T,
		typename = std::enable_if_t<std::is_base_of<IAstTag, typename T::TagType>::value>>
	typename T::TagType* query(AstNode* node);
};

template<typename T, typename Enable>
typename T::TagType*
AstTags::query(AstNode* node)
{
	auto node_iter = lut.find(node);
	if( node_iter == lut.end() )
	{
		lut.emplace(std::make_pair(node, std::map<int, std::unique_ptr<IAstTag>>{}));
		node_iter = lut.find(node);
	}

	auto type_iter = node_iter->second.find(typeid(T).hash_code());
	if( type_iter == node_iter->second.end() )
	{
		node_iter->second.emplace(std::make_pair(typeid(T).hash_code(), new typename T::TagType));
		type_iter = node_iter->second.find(typeid(T).hash_code());
	}

	auto baseptr = type_iter->second.get();
	return dynamic_cast<typename T::TagType*>(baseptr);
}
} // namespace ast