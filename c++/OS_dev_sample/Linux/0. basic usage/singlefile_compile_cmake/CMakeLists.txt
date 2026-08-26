cmake_minimum_required(VERSION 3.10)

# 프로젝트 이름 설정
project(MyFirstProject)

# C++ 표준 설정 (C++17 사용)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 실행 파일 생성 (빌드 결과물 이름: my_app, 소스 파일: main.cpp)
add_executable(my_app main.cpp)