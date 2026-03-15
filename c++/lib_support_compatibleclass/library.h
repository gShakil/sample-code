#pragma once
#include <iostream>


namespace ns_library
{
	class library
	{
	public:
		library()
		{
			std::cout << "call default construct" << std::endl;
		}
		template <typename compatibleclass>
		library(compatibleclass& t)
		{
			std::cout << "call custom construct" << std::endl;
			t.to_library(*this, t);
		};
		template <typename compatibleclass>
		library operator=(compatibleclass& t)
		{
			std::cout << "call custom operator" << std::endl;
			library lib;
			t.to_library(lib, t);
			return lib;
		}

		int x, y, w, h;
	};
}


class library_BaseClass
{
public:
	void from_library(ns_library::library& t, library_BaseClass& base)
	{
		std::cout << "Base class convert call !" << std::endl;
	}

	int base_a, base_b;
};