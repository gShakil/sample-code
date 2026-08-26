## cmake를 사용한 수동 컴파일 예제
 간단한 소스 작성 (./main.cpp)
```c++
// main.cpp

#include <iostream>
#include <vector>
#include <string>

int main()
{
    std::string s = "Hello World !";
    std::cout << s << std::endl;

    return 0;
}

```

CMakeLists.txt 작성
```c++
cmake_minimum_required(VERSION 3.10)

# 프로젝트 이름 설정
project(MyFirstProject)

# C++ 표준 설정 (C++17 사용)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 실행 파일 생성 (빌드 결과물 이름: my_app, 소스 파일: main.cpp)
add_executable(my_app main.cpp)
```

아래 명령어로 구성 파일 생성
```sh
mkdir build && cd build && cmake ..
```

아래 명령어로 컴파일 및 빌드
build 폴더 내부에서 실행
```sh
cmake --build .
```

build 폴더 내부에 my_app이란 파일 생성되었음을 확인.
