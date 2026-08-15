#pragma once
#include <stdint.h>

#if defined(_MSVC_LANG)
#define CURRENT_CPP_VERSION _MSVC_LANG
#else
#define CURRENT_CPP_VERSION __cplusplus
#endif

// C++14 여부 확인. 컴파일러 옵션: /Zc:__cplusplus 필요
#if CURRENT_CPP_VERSION >= 201402L
#define CONSTEXPR_BYTE_LITERAL constexpr
#else
#define CONSTEXPR_BYTE_LITERAL
#endif


namespace byte_literals
{
	CONSTEXPR_BYTE_LITERAL uint64_t operator"" _kb(uint64_t bytes) noexcept
	{
		return bytes * 1024;
	}
	CONSTEXPR_BYTE_LITERAL uint64_t operator"" _mb(uint64_t bytes) noexcept
	{
		return bytes * 1024 * 1024;
	}
	CONSTEXPR_BYTE_LITERAL uint64_t operator"" _gb(uint64_t bytes) noexcept
	{
		return bytes * 1024 * 1024 * 1024;
	}
}