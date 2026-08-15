#pragma once
#include <Windows.h>
#include <cstdint>
#include <type_traits>

class mono_clock_qpc {
public:
	using rep = std::int64_t;

	struct sec {};
	struct millisec {};
	struct microsec {};
	struct nanosec {};

	static rep now_ticks() noexcept {
		LARGE_INTEGER v;
		::QueryPerformanceCounter(&v);
		return v.QuadPart;   // monotonic raw tick
	}

	static rep frequency() noexcept {
		static const rep f = [] {
			LARGE_INTEGER v;
			::QueryPerformanceFrequency(&v);
			return static_cast<rep>(v.QuadPart);
		}();
		return f;
	}

	template <typename timetag>
	static double delta_to(rep later, rep earlier) noexcept {
		const double ticks = static_cast<double>(later - earlier);
		const double freq = static_cast<double>(frequency());
		if constexpr (std::is_same_v<timetag, sec>) {
			return ticks / freq;
		}
		else if constexpr (std::is_same_v<timetag, millisec>) {
			return (ticks * 1'000.0) / freq;
		}
		else if constexpr (std::is_same_v<timetag, microsec>) {
			return (ticks * 1'000'000.0) / freq;
		}
		else if constexpr (std::is_same_v<timetag, nanosec>) {
			return (ticks * 1'000'000'000.0) / freq;
		}
		else {
			static_assert(sizeof(timetag) == 0, "Unsupported");
		}
	}
};