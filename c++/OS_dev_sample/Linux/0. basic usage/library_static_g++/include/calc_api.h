#pragma once

#if defined(_WIN32)
#  ifdef CALC_BUILDING
#    define CALC_API __declspec(dllexport)
#  else
#    define CALC_API __declspec(dllimport)
#  endif
#else
#  define CALC_API __attribute__((visibility("default")))
#endif