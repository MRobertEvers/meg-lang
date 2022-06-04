#include <iostream>
#include <string>

extern "C" int test_sushi();

/**
 * @brief Used until sushi can generate main entry point code.
 *
 * @return int
 */
int
main()
{
	std::cout << "" << test_sushi() << std::flush;
	return 0;
}