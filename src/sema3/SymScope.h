#pragma once

#include <map>
#include <string>

class Sym;

// For some symbols,
// the symscope contains a flat lookup of contained symbols
class SymScope
{
	std::map<std::string, Sym*> syms;

public:
	Sym* find(std::string const& name);
	void insert(std::string const& name, Sym*);

	auto begin() { return syms.begin(); }
	auto end() { return syms.end(); }
};