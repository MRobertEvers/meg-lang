#include "utils.h"

#include "reverse.h"

int
count_end_whitespace(String const& s)
{
	int end_space = 0;
	for( auto& c : reverse(s) )
	{
		if( c == ' ' )
		{
			end_space += 1;
		}
		else
		{
			break;
		}
	}

	return end_space;
}