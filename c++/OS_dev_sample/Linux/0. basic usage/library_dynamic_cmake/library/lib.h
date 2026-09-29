#ifndef SHARED_LIB_H
#define SHARED_LIB_H

// 윈도우 환경(MSVC)에서 DLL 내보내기/가져오기 설정
#if defined(_WIN32) || defined(__CYGWIN__)
  #ifdef EXPORT_LIB
    #define LIB_API __declspec(dllexport)
  #else
    #define LIB_API __declspec(dllimport)
  #endif
#else
  #define LIB_API
#endif

extern "C" LIB_API void print_hello();

#endif