#pragma once

#include "ir/TypeInstance.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace ir
{

class NameId
{
	int index_;

public:
	NameId(int index)
		: index_(index)
	{}

	int index() const { return index_; }
};

class Name
{
public:
	enum NameKind
	{
		Member,
		Namespace,
		Type
	};

private:
	//
	std::string name_;
	std::map<std::string, NameId> lookup_;
	TypeInstance type_;

	NameKind kind_;

public:
	Name(std::string name)
		: name_(name)
		, kind_(NameKind::Namespace){};
	Name(std::string name, TypeInstance type, NameKind kind)
		: name_(name)
		, type_(type)
		, kind_(kind){};

	NameKind kind() { return kind_; }

	std::string name_str() const { return name_; }

	bool is_type() const { return kind_ == NameKind::Type; }
	bool is_member() const { return kind_ == NameKind::Member; }
	bool is_namespace() const { return kind_ == NameKind::Namespace; }

	TypeInstance type() const
	{
		assert(kind_ != NameKind::Namespace);
		return type_;
	}

	std::optional<NameId> lookup(std::string name);
	void add_name(std::string, NameId);
};

class NameRef
{
	NameId id_;
	std::vector<Name>* names_;

public:
	NameRef(std::vector<Name>* names, NameId id)
		: names_(names)
		, id_(id)
	{}

	Name& name() const { return names_->at(id_.index()); }
	Name& operator->() { return name(); }

	std::optional<NameRef> lookup(std::string) const;
	NameRef add_name(Name);
	TypeInstance type() const { return name().type(); }
};
} // namespace ir