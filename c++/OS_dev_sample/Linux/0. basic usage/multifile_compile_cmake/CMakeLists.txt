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