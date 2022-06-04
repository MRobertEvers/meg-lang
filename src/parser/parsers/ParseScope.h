#include "common/String.h"

#include <map>

namespace parser
{
class Identifier
{
public:
	String name;
	Identifier(String name)
		: name(name)
	{}
};

/**
 * For the most part, the parser does not need to keep track of scopes (yet),
 * however, when performing explicit template argument parsing, we can't know
 * if the parse is
 *
 * identifier < identifier
 *
 * or
 *
 * identifier<template_argument>
 *
 */
class ParseScope
{
	ParseScope* parent = nullptr;

	std::map<String, Identifier> names;

public:
	ParseScope(){};
	ParseScope(ParseScope* parent)
		: ParseScope(parent){};

	void add_named_value(String const& name, Identifier id);
	Identifier* get_identifier(std::string const& name);

	ParseScope* get_parent() { return parent; }
};
} // namespace parser