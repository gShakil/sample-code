# 파일 정의

``` c++
// ./include/calc.h
#pragma once
#include <string>

namespace nscalc{
    double add(double a, double b);
    double divide(double a, double b);
    std::string describe(double v);
}
```
``` c++
// ./src/calc.cpp
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
``` c++
// ./src/main.cpp
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

# ./CMakeLists.txt 작성
| 상황 | 필요 동작 |
| :--- | ---: |
| .cpp, .h 내용만 수정 | cmake --buile build -j 만 |
| 새 .cpp 파일 추가 | CMakeLists.txt의 add_executable 목록에 파일명 추가 -> 빌드하면 configure는 자동으로 다시 돎 |
| 새 .h 파일 추가 | 필요 동작 없음|
| CMakeLists.txt 수정 | 빌드만 동작 (자동 재구성)|
| 알 수 없는 오류| rm -rf build 후 재구성|

``` bash
# 최소 CMake 버전. 첫 줄 필수
cmake_minimum_required(VERSION 3.16)
# 프로젝트 이름 선언. CXX: c++만 사용. 없으면 c 컴파일러까지 검색.
project(calcdemo LANGUAGES CXX)
#g++ 사용시 -std=c++17 추가
set(CMAKE_CXX_STANDARD 17)
#c++17 지원하지 않으면 실패. 없다면 하위 버전으로 낮춤.
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# build/compile_commands.json 생성.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# calc라는 타겟을 이 소스로 생성.
add_executable(calc
    src/main.cpp
    src/calc.cpp
)
# Iinclude 추가
target_include_directories(calc PRIVATE include)
# 경고 수준
target_compile_options(calc PRIVATE -Wall -Wextra)
```

## 폴더 구조 확인
``` bash
.:
total 8
-rw-r--r-- 1 ksw ksw    0 Aug 17 15:44 CMakeLists.txt
drwxr-xr-x 2 ksw ksw 4096 Aug 17 15:40 include
drwxr-xr-x 2 ksw ksw 4096 Aug 17 15:44 src

./include:
total 4
-rw-r--r-- 1 ksw ksw 162 Aug 17 15:40 calc.h

./src:
total 8
-rw-r--r-- 1 ksw ksw 440 Aug 17 15:40 calc.cpp
-rw-r--r-- 1 ksw ksw 319 Aug 17 15:40 main.cpp
```

# 구성 명령어 입력. (한 번만)
``` bash
// -S . : 소스 디렉터리를 현재 폴더로. (CMakeLists.txt가 있는 곳)
// -B build : 빌드 산출물을 build/ 폴더에 지정
// -DCMAKE_BUILD_TYPE=Debug : 이 구문 없으면 최적화도, 디버그 심볼도 없이 생성됨. CMake는 기본 빌드 타입 없음.
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```

## 출력 문구 확인
``` bash
-- The CXX compiler identification is GNU 11.4.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done
-- Generating done
-- Build files have been written to: /home/ksw/260816_linux/files_compile_cmake/build
```

# 빌드 및 실행
```bash
// --build: 빌드 모드로 전환.
// build: 빌드 디렉터리 (cmake -S . -B build 에서 만든 것)
// -j : CPU 코어 수 만큼 병렬 컴파일
cmake --build build -j
```

``` bash
./build/calc
```




