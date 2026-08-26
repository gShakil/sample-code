# g++를 사용한 여러 파일 컴파일

## 파일 정의
```c++
// main.cpp
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
```

```c++
// calc.h
#pragma once
#include <string>

namespace nscalc{
    double add(double a, double b);
    double divide(double a, double b);
    std::string describe(double v);
}
```

```c++
// calc.cpp
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
```

## 1. 컴파일
> 컴파일 옵션 설명은 info.md 참조
``` bash
g++ -std=c++17 -Wall -Wextra -g -Iinclude main.cpp calc.cpp -o calc
```

## 2. 파일 확인
``` bash
-rwxr-xr-x 1 ksw ksw 94376 Aug 16 21:27 calc
-rw-r--r-- 1 ksw ksw   440 Aug 16 21:27 calc.cpp
-rw-r--r-- 1 ksw ksw   162 Aug 16 21:23 calc.h
-rw-r--r-- 1 ksw ksw   319 Aug 16 21:27 main.cpp
```

## 3. 실행, 출력 확인
``` bash
./calc
```

``` c++
결과: 7
예외: 0으로 나눌 수 없습니다.
```
