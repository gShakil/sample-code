#include "compatible.h"
#include "library.h"

void compatible::to_library(ns_library::library& lib, compatible& comp)
{
	std::cout << "compatible_to_library" << std::endl;
}

void compatible::from_library(ns_library::library& lib, compatible& comp)
{
	std::cout << "compatible_from_library" << std::endl;
}
