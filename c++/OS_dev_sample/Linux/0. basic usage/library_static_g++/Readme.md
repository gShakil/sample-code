# 정적 라이브러리 (.a) 생성 및 사용 방법
## 사전 파일 정의
```c++
// include/calc.h
#pragma once
#include <string>

namespace nscalc {
    double add(double a, double b);
    double divide(double a, double b);
    std::string describe(double v);
    const char* version();
}
```
``` c++
// src/calc.cpp
#include "calc.h"
#include <sstream>
#include <stdexcept>

namespace nscalc {

double add(double a, double b) {
    return a + b;
}

double divide(double a, double b) {
    if (b == 0.0) {
        throw std::domain_error("0으로 나눌 수 없습니다");
    }
    return a / b;
}

std::string describe(double v) {
    std::ostringstream os;
    os << "결과 = " << v;
    return os.str();
}

const char* version() {
    return "v1";
}

} // namespace nscalc
```
``` c++
#include <iostream>
#include "calc.h"

int main() {
    std::cout << "libcalc " << nscalc::version() << '\n';
    std::cout << nscalc::describe(nscalc::add(3, 4)) << '\n';
    return 0;
}
```

## 정적 라이브러리 (.a) 생성
### 1. 오브젝트 컴파일
>-fvisibility=hidden : 기본값을 노출시키지 않는 것으로 설정. (함수 기본값을 노출않는 것으로.) 
>-fvisibility-inlines-hidden: inline 함수만 추가로 hidden으로. 
>-DCALC_BUILDING : \#define CALC_BUILDING 1 과 동일.
``` bash
g++ -std=c++17 -Wall -Wextra -g -Iinclude -fvisibility=hidden -fvisibility-inlines-hidden -DCALC_BUILDING -c src/calc.cpp -o build/calc.o
```
### 2. 아카이브 묶기
> r: 파일 추가/교체
> c: 아카이브 없는 경우 생성
> s: 심볼 인덱스 생성 (=ranlib)
``` bash
ar rcs lib/libcalc.a build/calc.o
```
내부 확인 방법
```bash
ar t lib/libcalc.a && nm -C --defined-only lib/libcalc.a | head -20
```
```
calc.o

calc.o:
0000000000000000 V DW.ref.__gxx_personality_v0
0000000000000000 T nscalc::add(double, double)
0000000000000028 T nscalc::divide(double, double)
00000000000001a6 T nscalc::version()
00000000000000cb T nscalc::describe[abi:cxx11](double)
```

### 3. 사용하기
> -Iinlucde : include 폴더를 추가 포함 디렉터리로 사용
> -Llib: lib 폴더를 추가 라이브러리 디렉터리 폴더로 사용
> -lcalc: 추가 종속성 정의. lib + calc + .a 로 조합해서 파일을 찾음. 접두사 lib와 확장자는 빼고 적음. 즉, -a 파일 이름은 lib로 시작해야 함. 같은 이름이 있는 경우 공유(so) -> 정적 (a) 우선
>> g++ -std=c++17 -I include app/main.cpp -L lib -l calc -o build/app_static 와 같은 의미
``` bash
g++ -std=c++17 -Iinclude src/main.cpp -Llib -lcalc -o bin/app_static
```