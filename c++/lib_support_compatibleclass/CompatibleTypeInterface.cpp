#include "library.h"
#include "compatible.h"




int main()
{
	ns_library::library default_creator;

	compatible comp;
	ns_library::library lib(comp);

	lib = comp;
}
