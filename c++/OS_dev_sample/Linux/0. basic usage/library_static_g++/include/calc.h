#pragma once
#include <string>
#include "calc_api.h"

namespace nscalc {
    CALC_API double add(double a, double b);
    CALC_API double divide(double a, double b);
    std::string describe(double v);
    const char* version();
}