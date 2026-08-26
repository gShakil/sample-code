# 동적 라이브러리 (.so) 생성 및 사용 방법
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

## 동적 라이브러리 (.a) 생성
### 1. fPIC로 컴파일
> -fPIC = Position Independent Code. 공유 라이브러리는 프로세스마다 다른 가상 주소에 매핑될 수 있어서, 절대 주소를 코드에 입력하면 안됨. 추가하지 않으면 아래 에러가 발생.
>> relocation R_X86_64_PC32 against symbol ... can not be used when making a shared object; recompile with -fPIC


> -fvisibility=hidden : 기본값을 노출시키지 않는 것으로 설정. (함수 기본값을 노출않는 것으로.)
> -fvisibility-inlines-hidden: inline 함수만 추가로 hidden으로.
> -DCALC_BUILDING : \#define CALC_BUILDING 1 과 동일.
``` bash
g++ -std=c++17 -Wall -Wextra -g -Iinclude -fPIC -fvisibility=hidden -fvisibility-inlines-hidden -DCALC_BUILDING -c src/calc.cpp -o build/calc.pic.o
```
### 2. 공유 라이브러리로 링크
```bash
g++ -shared build/calc.pic.o -o lib/libcalc.so
```

### 3. 확인 방법
```bash
file lib/libcalc.so && nm -D -C lib/libcalc.so | grep nscalc
```
```c++
// nm -D: 동적 심볼 테이블 조회 명령. (= dumpbin /exports)
// .def, dllexport도 없는데 nscalc:add 등 함수가 노출됨. 
// 리눅스는 기본이 전체 공개이기 때문.

lib/libcalc.so: ELF 64-bit LSB shared object, x86-64, version 1 (SYSV), dynamically linked, BuildID[sha1]=e36856e89c8a54cdf677de115f397528ecfc39c5, with debug_info, not stripped
0000000000001259 T nscalc::add(double, double)
0000000000001281 T nscalc::divide(double, double)
00000000000013ff T nscalc::version()
0000000000001324 T nscalc::describe[abi:cxx11](double)
```

## 공유 라이브러리 사용.
### 1. 빌드
```bash
g++ -std=c++17 -Iinclude src/main.cpp -Llib -lcalc -o bin/app_shared
```
### 2. 실행 (오류 발생)
> 경로 미지정시 다음과 같은 오류가 발생.
> 빌드 시점 경로(-L)와 실행 시점 경로는 별개. (= 윈도우에서 dll을 exe와 같이 두지 않았을 때 나오는 오류)
```
./bin/app_shared: error while loading shared libraries: libcalc.so: cannot open shared object file: No such file or directory
```

### 3. 문제 진단
``` bash
ldd bin/app_shared 
```
``` c++
        linux-vdso.so.1 (0x00007e554afd4000)
        libcalc.so => not found  //  !!! 문제 발생 !!!
        libstdc++.so.6 => /lib/x86_64-linux-gnu/libstdc++.so.6 (0x00007e554ac00000)
        libgcc_s.so.1 => /lib/x86_64-linux-gnu/libgcc_s.so.1 (0x00007e554af9f000)
        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007e554a800000)
        libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007e554aeb8000)
        /lib64/ld-linux-x86-64.so.2 (0x00007e554afd6000)
```
### 4. 문제 해결 (rpath 설정)
> -W1, ... : 뒤 내용을 링커(ld)에게 그대로 전달
> \$ORIGIN: 실행 파일 자신이 있는 디렉터리 (런타임에 해석)
> 작은 따옴표: 없으면 \$ORIGIN을 빈 문자열로 치환
>> \$ORIGIN\..\lib = bin\..\lib = lib\. 폴더째 어디로 옮겨도 동작. (= 윈도우의 dll을 exe 옆에 위치시키는 것.)
```bash
g++ -std=c++17 -Iinclude src/main.cpp -Llib -lcalc -Wl,-rpath,'$ORIGIN/../lib' -o bin/app_shared
```
### 5. 문제 해결 확인
```bash
readelf -d bin/app_shared | grep -i -E 'rpath|runpath'
```
출력: 
```
0x000000000000001d (RUNPATH)            Library runpath: [$ORIGIN/../lib]
```
