#include "ParseResult.h"

#include "common/String.h"

#include <iostream>
#include <string.h>

String
get_line(char const* line)
{
	auto offset = strstr(line + 1, "\n");
	if( offset == nullptr )
	{
		return "";
	}
	unsigned int size = offset - line;
	if( size == 1 )
		return "";
	return String(line + 1, size - 1);
}

void
ParseError::print() const
{
	std::cout << "Parse Error:\n";
	std::cout << "\t" << error << "\n";

	int line_start = token.neighborhood.line_num - 1;
	if( line_start < 0 )
	{
		line_start = 0;
	}
	if( token.neighborhood.lines.num_lines == 0 )
		return;

	int line_end = token.neighborhood.line_num + 1;
	if( line_end >= token.neighborhood.lines.num_lines )
	{
		line_end = token.neighborhood.lines.num_lines - 1;
	}

	for( int i = line_start; i <= line_end; i++ )
	{
		auto line = get_line(token.neighborhood.lines.lines[i]);

		auto ln_str = std::to_string(i);
		std::cout << ln_str << " | " << line << "\n";

		if( i == token.neighborhood.line_num )
		{
			auto sz = String{token.start, token.size};
			char const* offset = strstr(line.c_str(), sz.c_str());
			assert(offset != nullptr);
			unsigned int diff = offset - line.c_str();

			std::cout << String(ln_str.size(), ' ') << " | " << String(diff, ' ')
					  << String(token.size, '^') << " here" << std::endl;
		}
	}
}