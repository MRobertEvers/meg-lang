#pragma once

#include <string>
#include <vector>

namespace sema
{
class QualifiedName
{
	std::vector<std::string> name_parts_;

public:
	QualifiedName(){};
	QualifiedName(std::string const& name) { name_parts_.push_back(name); };

	int length() const { return name_parts_.size(); }
	std::string part(int id) const { return name_parts_[id]; }

	void add_part(std::string const& part) { name_parts_.push_back(part); }

	std::string to_string() const
	{
		std::string res;

		for( auto part : name_parts_ )
			res += "::" + part;

		return res;
	}
};

// class SimpleName
// {
// 	std::string simple_;

// public:
// 	SimpleName(){};
// 	SimpleName(std::string const& name)
// 		: simple_(name){};

// 	std::string to_string() const { return simple_; }
// }
} // namespace sema