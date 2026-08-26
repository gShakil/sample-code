#include "calc.h"
#include <sstream>
#include <stdexcept>

namespace nscalc{
    double add(double a, double b)    {
        return a + b;
    }

    double divide(double a, double b){
        if(b == 0.0)
            throw std::domain_error("0으로 나눌 수 없습니다.");
        return a / b;
    }

    std::string describe(double v){
        std::ostringstream os;
        os << "결과: " << v;
        return os.str();
    }
}

