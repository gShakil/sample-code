#include <iostream>
#include "calc.h"

int main()
{
    std::cout << nscalc::describe(nscalc::add(3, 4)) << '\n';

    try
    {
        std::cout << nscalc::describe(nscalc::divide(10,0)) << '\n';
    }
    catch(const std::exception& e)
    {
        std::cerr << "예외: " <<e.what() << '\n';
    }

    return 0;
}