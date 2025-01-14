#include "string_literals.h"

int main()
{
	using namespace custom_literals;

	int Num = "operator_test"_toint; // string call tolib. [operator_test]
	R"(operator_void__\n\t\r)"_tovoid; // string call tolib. [operator_void__\n\t\r]size: 21
	R"START_DELIMITER(operator_void__\n\t\r)START_DELIMITER"_tovoid; // string call tolib. [operator_void__\n\t\r]size: 21
}