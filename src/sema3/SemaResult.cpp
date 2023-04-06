#include "SemaResult.h"

#include <iostream>
#include <string>

void
SemaError::print() const
{
	std::cout << "Sema Error:\n";
	std::cout << "\t" << error << "\n";
}