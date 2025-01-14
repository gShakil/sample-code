#include <variant>
#include <string>
#include <iostream>

struct AddressPrinter
{
	void operator() (const std::string& s) const
	{
		std::cout << "A string call :" << s << std::endl;
	}
	void operator() (const int& n) const
	{
		std::cout << "A int call :" << n << std::endl;
	}
};

int main()
{
	// c++ 17 required
	std::variant<std::string, int> var;

	var = "string example";
	
	AddressPrinter ap;
	std::visit(ap, var);
	// std::cout << "A string call :" << s << std::endl;

	var = 777;
	std::visit(ap, var);
	// std::cout << "A int call : :" << s << std::endl;
}