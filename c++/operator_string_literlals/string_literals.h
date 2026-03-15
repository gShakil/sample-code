#pragma once
#include <iostream>

namespace custom_literals
{
	int operator ""_toint(const char* s, std::size_t n)
	{
		std::cout << "string call tolib. [" << s << "]" << std::endl;
		return 5;
	}

	void operator ""_tovoid(const char* s, std::size_t n)
	{
		std::cout << "string call tolib. [" << s << "]" << "size: " << n << std::endl;
	}
}