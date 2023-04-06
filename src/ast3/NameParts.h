#pragma once

#include <string>
#include <vector>

struct NameParts
{
	std::vector<std::string> parts;

	NameParts(std::vector<std::string> parts)
		: parts(parts){};
};