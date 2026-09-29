# 이 cmake 파일을 처리하기 위한 최소 cmake 버전
cmake_minimum_required(VERSION 3.10)
# 프로젝트 이름 지정
project(staticLibrarySample)


# staticlib이라는 이름의 정적 라이브러리 타깃 생성
# 윈도우에서는 dll, 리눅스에서는 so 생성
# 소스 코드는 library/lib.cpp 사용
add_library(staticlib STATIC library/lib.cpp)
# staticlib 라이브러리가 컴파일될 때, 그리고 이 라이브러리를 사용하는 다른 대상이
# 참조할 헤더 파일의 검색 경로 지정.
# 키워드로 PRIVATE / PUBLIC / INTERFACE 존재.
# PUBLIC 으로 지정시 이 라이브러리를 링크하는 곳에서도 이 설정을 반영함.
target_include_directories(staticlib PUBLIC library)



# 실행 파일 생성 및 라이브러리 링크
add_executable(app src/main.cpp)
# app 실행 파일이 staticlib 라이브러리를 링크하도록.
target_link_libraries(app PRIVATE staticlib)