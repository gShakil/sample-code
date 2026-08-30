cmake_minimum_required(VERSION 3.10)
project(avn_ivi_sim CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(avn_sim src/main.cpp)

if(MSVC)
    # /utf-8: 소스의 한글 주석을 UTF-8로 해석
    target_compile_options(avn_sim PRIVATE /W4 /utf-8)
else()
    target_compile_options(avn_sim PRIVATE -Wall -Wextra)
endif()
