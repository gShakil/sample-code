# 이 cmake 파일을 처리하기 위한 최소 cmake 버전
cmake_minimum_required(VERSION 3.10)
# 프로젝트 이름 지정
project(dynamicLibrarySample)


# dynamiclib이라는 이름의 동적 라이브러리 타깃 생성
# 윈도우에서는 dll, 리눅스에서는 so 생성
# 소스 코드는 library/lib.cpp 사용
add_library(dynamiclib SHARED library/lib.cpp)
# dynamiclib 라이브러리가 컴파일될 때, 그리고 이 라이브러리를 사용하는 다른 대상이
# 참조할 헤더 파일의 검색 경로 지정.
# 키워드로 PRIVATE / PUBLIC / INTERFACE 존재.
# PUBLIC 으로 지정시 이 라이브러리를 링크하는 곳에서도 이 설정을 반영함.
target_include_directories(dynamiclib PUBLIC library)
# 윈도우에서 심볼 내보내기를 위해 매크로 정의
# PRIVATE: 이 라이브러리를 빌드할 때에만 매크로가 적용됨.
target_compile_definitions(dynamiclib PRIVATE EXPORT_LIB)
# 실행 파일과 동일한 디렉터리에서 .so 파일을 찾도록 설정



# 실행 파일 생성 및 라이브러리 링크
add_executable(app src/main.cpp)
# app 실행 파일이 dynamiclib 라이브러리를 링크하도록.
target_link_libraries(app PRIVATE dynamiclib)
# app 실행 파일을 컴파일할 때 library 디렉토리를 헤더 검색 경로에 추가한다.

target_include_directories(app PRIVATE library)




# install 옵션 설정
# 아래 두 줄이 없다면 라이브러리 링크 폴더가 빌드할 때의 절대 경로가 박힘.
# 만약 실행 파일은 ./build 폴더 내부에 있고, 라이브러리는 ./lib/에 있다면 $ORIGIN/../lib 로 선언
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
# 빌드할 때도 위 RPATH 사용하여 install 버전의 RPATH($ORIGIN/../lib)를 사용하고자 한다면 아래 구문 주석 해제.
# 기본적으로 빌드할 때와, 배포할 때 참조 경로가 다름.
# set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

# 어떤 파일을 어디로 복사(설치)할지 규칙 정의
#    - RUNTIME(실행 파일): 디렉터리 내 bin/ 폴더로
#    - LIBRARY(동적 라이브러리): 디렉터리 내 lib/ 폴더로
install(TARGETS app RUNTIME DESTINATION bin)
install(TARGETS dynamiclib LIBRARY DESTINATION lib)