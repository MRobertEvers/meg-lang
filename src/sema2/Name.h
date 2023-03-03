#pragma once

#include "MemberTypeInstance.h"
#include "TypeInstance.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace sema
{

class NameId
{
	int index_;

public:
	NameId() = default;
	NameId(int index)
		: index_(index)
	{}

	int index() const { return index_; }
};

class Name
{
public:
	enum class NameKind
	{
		Var,
		Member,
		Namespace,
		Type
	};

private:
	//
	std::string name_;
	std::map<std::string, NameId> lookup_;
	std::optional<NameId> parent_;

	union
	{
		TypeInstance type_;
		MemberTypeInstance member_type_;
	};

	NameKind kind_;

public:
	Name(std::string name)
		: name_(name)
		, kind_(NameKind::Namespace){};
	Name(std::string name, NameId parent)
		: name_(name)
		, parent_(NameId(parent))
		, kind_(NameKind::Namespace){};
	Name(std::string name, TypeInstance type, NameKind kind)
		: name_(name)
		, type_(type)
		, kind_(kind){};
	Name(std::string name, NameId parent, TypeInstance type, NameKind kind)
		: name_(name)
		, parent_(NameId(parent))
		, type_(type)
		, kind_(kind){};

	NameKind kind() { return kind_; }

	std::string name_str() const { return name_; }

	bool is_type() const { return kind_ == NameKind::Type; }
	bool is_member() const { return kind_ == NameKind::Member; }
	bool is_var() const { return kind_ == NameKind::Var; }
	bool is_namespace() const { return kind_ == NameKind::Namespace; }

	TypeInstance type() const
	{
		assert(kind_ != NameKind::Namespace);
		return type_;
	}

	std::optional<NameId> parent();
	std::optional<NameId> lookup(std::string name);
	void add_name(std::string, NameId);

	static Name Var(std::string name, NameId parent, TypeInstance type)
	{
		return Name(name, parent, type, NameKind::Var);
	}

	static Name Type(std::string name, NameId parent, TypeInstance type)
	{
		return Name(name, parent, type, NameKind::Type);
	}
};

class NameRef
{
	NameId id_;
	std::vector<Name>* names_;

public:
	NameRef() = default;
	NameRef(std::vector<Name>* names, NameId id)
		: names_(names)
		, id_(id)
	{}

	Name& name() const { return names_->at(id_.index()); }
	Name& operator->() { return name(); }

	std::optional<NameRef> lookup(std::string) const;
	NameRef add_name(Name);
	TypeInstance type() const { return name().type(); }
	NameId id() const { return id_; }

	std::string to_fqn_string() const;
};
} // namespace sema