#include <iostream>
#include "calc.h"

int main() {
    std::cout << "libcalc " << nscalc::version() << '\n';
    std::cout << nscalc::describe(nscalc::add(3, 4)) << '\n';
    return 0;
}