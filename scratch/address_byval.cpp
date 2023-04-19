
#include <iostream>

struct Point
{
	int x;
	int y;
	int z[5];
};

Point
add(int q, Point a, Point b, int i)
{
	int first = 8;
	std::cout << "Hello" << std::endl;
	std::cout << (long)(void*)&q << std::endl;
	std::cout << (long)(void*)&a << std::endl;
	std::cout << (long)(void*)&b << std::endl;
	std::cout << (long)(void*)&i << std::endl;
	std::cout << (long)(void*)&first << std::endl;

	return Point{.x = 1};
}

int
main()
{
	Point p1;
	Point p2;
	int i = 0;
	std::cout << (long)(void*)&i << std::endl;
	add(5, p1, p2, i);

	return 0;
}