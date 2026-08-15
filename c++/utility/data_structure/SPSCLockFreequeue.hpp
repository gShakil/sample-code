#pragma once
/**
 * SPSC FIFO lock-free bounded ring buffer.
 *
 * - 정확히 한 스레드만 push, 한 스레드만 pop 해야 함(위반 시 데이터 레이스).
 * - 용량은 bufCount 이상으로 2의 거듭제곱으로 올림(인덱스 마스킹).
 * - try_push(T&&): noexcept 이동 생성 권장(재시도 시 동일 객체를 다시 이동하지 않음).
 * - try_pop(T&): noexcept 이동 대입 권장.
 */

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <new>
#include <type_traits>
#include <utility>

namespace spsc_queue_detail
{
	inline std::size_t max_pow2_size_t() noexcept
	{
		std::size_t p = 1;
		while (p <= (std::numeric_limits<std::size_t>::max)() / 2)
			p <<= 1;
		return p;
	}

	inline std::size_t ceil_pow2(std::size_t n)
	{
		if (n <= 1)
			return 1;
		const std::size_t hi = max_pow2_size_t();
		if (n > hi)
			return hi;
		std::size_t p = 1;
		while (p < n)
			p <<= 1;
		return p;
	}
} // namespace spsc_queue_detail

template <typename T>
class SPSCLockFreeQueue
{
	static_assert(std::is_nothrow_destructible<T>::value, "T must be nothrow destructible.");

private:
	struct alignas(64) Cell
	{
		alignas(T) unsigned char storage[sizeof(T)];
		Cell() noexcept = default;
	};

	static T* launder_ptr(void* p) noexcept
	{
#if defined(__cpp_lib_launder) && __cpp_lib_launder >= 201606L
		return std::launder(reinterpret_cast<T*>(p));
#else
		return reinterpret_cast<T*>(p);
#endif
	}

	void discard_all() noexcept
	{
		for (;;)
		{
			const std::uint64_t r = m_readIdx.load(std::memory_order_relaxed);
			const std::uint64_t w = m_writeIdx.load(std::memory_order_acquire);
			if (r >= w)
				return;

			Cell& cell = m_cells[r & m_mask];
			launder_ptr(cell.storage)->~T();
			m_readIdx.store(r + 1, std::memory_order_release);
		}
	}

	std::size_t m_capacity{};
	std::size_t m_mask{};
	alignas(64) std::atomic<std::uint64_t> m_readIdx{};
	alignas(64) std::atomic<std::uint64_t> m_writeIdx{};
	Cell* m_cells{ nullptr };

public:
	/// bufCount: 최소 슬롯 수(내부에서 2의 거듭제곱으로 올림). 0이면 1로 취급.
	explicit SPSCLockFreeQueue(std::size_t bufCount)
		: m_capacity(spsc_queue_detail::ceil_pow2(bufCount == 0 ? 1 : bufCount))
		, m_mask(m_capacity - 1)
	{
		m_cells = static_cast<Cell*>(::operator new[](sizeof(Cell) * m_capacity, std::align_val_t{ alignof(Cell) }));
		for (std::size_t i = 0; i < m_capacity; ++i)
			::new (m_cells + i) Cell();
	}

	SPSCLockFreeQueue(const SPSCLockFreeQueue&) = delete;
	SPSCLockFreeQueue& operator=(const SPSCLockFreeQueue&) = delete;
	SPSCLockFreeQueue(SPSCLockFreeQueue&&) = delete;
	SPSCLockFreeQueue& operator=(SPSCLockFreeQueue&&) = delete;

	~SPSCLockFreeQueue() noexcept
	{
		discard_all();
		for (std::size_t i = 0; i < m_capacity; ++i)
			m_cells[i].~Cell();
		::operator delete[](m_cells, std::align_val_t{ alignof(Cell) });
		m_cells = nullptr;
	}

	std::size_t capacity() const noexcept { return m_capacity; }

	[[nodiscard]] bool isEmpty() const noexcept
	{
		return m_readIdx.load(std::memory_order_relaxed) == m_writeIdx.load(std::memory_order_relaxed);
	}

	[[nodiscard]] bool isFull() const noexcept
	{
		const std::uint64_t w = m_writeIdx.load(std::memory_order_relaxed);
		const std::uint64_t r = m_readIdx.load(std::memory_order_acquire);
		return (w - r) >= m_capacity;
	}

	[[nodiscard]] std::size_t getFreeCount() const noexcept
	{
		const std::uint64_t w = m_writeIdx.load(std::memory_order_relaxed);
		const std::uint64_t r = m_readIdx.load(std::memory_order_acquire);
		const std::uint64_t used = w - r;
		return used >= m_capacity ? 0 : static_cast<std::size_t>(m_capacity - used);
	}

	[[nodiscard]] bool try_push(const T& value)
	{
		T tmp(value);
		return try_push(std::move(tmp));
	}

	[[nodiscard]] bool try_push(T&& value) noexcept(std::is_nothrow_move_constructible<T>::value)
	{
		static_assert(std::is_nothrow_move_constructible<T>::value, "try_push(T&&) requires noexcept move construction.");

		const std::uint64_t w = m_writeIdx.load(std::memory_order_relaxed);
		const std::uint64_t r = m_readIdx.load(std::memory_order_acquire);
		if (w - r >= m_capacity)
			return false;

		Cell& cell = m_cells[w & m_mask];
		::new (cell.storage) T(std::move(value));
		m_writeIdx.store(w + 1, std::memory_order_release);
		return true;
	}

	template <class... Args>
	[[nodiscard]] bool try_emplace(Args&&... args)
	{
		T tmp(std::forward<Args>(args)...);
		return try_push(std::move(tmp));
	}

	[[nodiscard]] bool try_pop(T& out) noexcept(std::is_nothrow_move_assignable<T>::value)
	{
		static_assert(std::is_nothrow_move_assignable<T>::value, "try_pop(T&) requires noexcept move assignment.");

		const std::uint64_t r = m_readIdx.load(std::memory_order_relaxed);
		const std::uint64_t w = m_writeIdx.load(std::memory_order_acquire);
		if (r >= w)
			return false;

		Cell& cell = m_cells[r & m_mask];
		T* const p = launder_ptr(cell.storage);
		out = std::move(*p);
		p->~T();
		m_readIdx.store(r + 1, std::memory_order_release);
		return true;
	}
};
