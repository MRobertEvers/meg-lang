#include "ParseResult.h"

#include <iostream>
#include <string>

static std::string
get_line(char const* line, int line_num)
{
	if( line_num != 0 )
		line += 1; // Skip passed '\n' character.
	auto offset = strstr(line, "\n");
	if( offset == nullptr )
	{
		return "";
	}
	unsigned int size = offset - line;
	if( size == 1 )
		return "";
	return std::string(line, size);
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
	if( line_end > token.neighborhood.lines.num_lines )
	{
		line_end = token.neighborhood.lines.num_lines;
	}

	for( int i = line_start; i <= line_end; i++ )
	{
		auto line = get_line(token.neighborhood.lines.lines[i], i);

		auto ln_str = std::to_string(i + 1);
		std::cout << ln_str << " | " << line << "\n";

		if( i == token.neighborhood.line_num )
		{
			auto sz = std::string{token.view.start, token.view.size};
			int offset = token.view.start - token.neighborhood.lines.lines[i];
			if( i != 0 )
				offset -= 1; // Skip passed '\n' character.
			assert(offset > 0);

			std::cout << std::string(ln_str.size(), ' ') << " | " << std::string(offset, ' ')
					  << std::string(token.view.size, '^') << " here" << std::endl;
		}
	}
}