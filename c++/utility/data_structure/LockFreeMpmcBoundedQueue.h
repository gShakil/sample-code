#pragma once

/**
 * Lock-free bounded MPMC queue (Vyukov / 1024cores 스타일, head/tail CAS).
 *
 * 계약(실무용):
 * - push/pop 모두 mutex/spinlock 없음.
 * - 동시에 여러 생산자·소비자 허용.
 * - 용량은 생성 시 지정한 최소 개수 이상으로 2의 거듭제곱으로 올림.
 * - 소멸자 호출 시 다른 스레드가 push/pop 하면 안 됨(정지 후 파괴).
 * - try_push(T&&): T 이동 생성이 noexcept 이어야 함(실패 시 재시도로 인해 동일 객체를 여러 번 이동하지 않음).
 * - try_push(const T&): 복사 생성은 CAS 전에 수행(복사 예외는 큐를 오염시키지 않음), 이후 noexcept 이동만 큐에 반영.
 * - try_pop(T&): T에 대한 이동 대입이 noexcept 이어야 함.
 * - drain_all: 슬롯에서 노출용 임시 버퍼로 noexcept 이동 생성 후 vector로 옮김(try_push와 동일하게 이동 생성 noexcept 필요).
 */

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
#include <immintrin.h>
#endif

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif

namespace enf::lockfree
{
	namespace detail
	{
		inline void cpu_pause() noexcept
		{
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
			_mm_pause();
#elif defined(_WIN32)
			YieldProcessor();
#else
			/* 다른 아키텍처는 필요 시 명령 추가 */
#endif
		}

		inline void spin_backoff(unsigned& spins) noexcept
		{
			if (spins < 12)
			{
				cpu_pause();
				++spins;
				return;
			}
			if (spins < 24)
			{
				for (int i = 0; i < 4; ++i)
					cpu_pause();
				++spins;
				return;
			}
#ifdef _WIN32
			::SwitchToThread();
#else
			cpu_pause();
#endif
			spins = 0;
		}

		inline std::size_t max_pow2_size_t() noexcept
		{
			std::size_t p = 1;
			while (p <= std::numeric_limits<std::size_t>::max() / 2)
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

		template <class T>
		inline T* launder_ptr(void* p) noexcept
		{
#if defined(__cpp_lib_launder) && __cpp_lib_launder >= 201606L
			return std::launder(reinterpret_cast<T*>(p));
#else
			return reinterpret_cast<T*>(p);
#endif
		}
	} // namespace detail

	template <class T>
	class MpmcBoundedQueue
	{
		static_assert(
			alignof(T) <= 64,
			"T alignment over 64 may need larger cell padding; adjust Cell if required.");

		static_assert(
			std::is_nothrow_destructible<T>::value,
			"T must be nothrow destructible for noexcept discard/ pop paths.");

	public:
		/// max_items: 논리적 최소 용량(내부에서 2의 거듭제곱으로 올림).
		explicit MpmcBoundedQueue(std::size_t max_items)
			: capacity_(detail::ceil_pow2(max_items))
			, mask_(capacity_ - 1)
		{
			cells_ = static_cast<Cell*>(::operator new[](sizeof(Cell) * capacity_, std::align_val_t{ alignof(Cell) }));
			for (std::size_t i = 0; i < capacity_; ++i)
			{
				::new (cells_ + i) Cell();
				cells_[i].sequence.store(i, std::memory_order_relaxed);
			}
			head_.store(0, std::memory_order_relaxed);
			tail_.store(0, std::memory_order_relaxed);
		}

		MpmcBoundedQueue(const MpmcBoundedQueue&) = delete;
		MpmcBoundedQueue& operator=(const MpmcBoundedQueue&) = delete;
		MpmcBoundedQueue(MpmcBoundedQueue&&) = delete;
		MpmcBoundedQueue& operator=(MpmcBoundedQueue&&) = delete;

		~MpmcBoundedQueue() noexcept
		{
			// 다른 스레드가 접근하지 않을 때만 호출.
			discard_all();
			for (std::size_t i = 0; i < capacity_; ++i)
				cells_[i].~Cell();
			::operator delete[](cells_, std::align_val_t{ alignof(Cell) });
			cells_ = nullptr;
		}

		std::size_t capacity() const noexcept { return capacity_; }

		/// 대략적 적재 개수(동시 접근 중에는 부정확할 수 있음).
		std::size_t size_approx() const noexcept
		{
			return head_.load(std::memory_order_relaxed) - tail_.load(std::memory_order_relaxed);
		}

		[[nodiscard]] bool try_push(const T& value)
		{
			T tmp(value);
			return try_push(std::move(tmp));
		}

		[[nodiscard]] bool try_push(T&& value) noexcept(std::is_nothrow_move_constructible<T>::value)
		{
			static_assert(
				std::is_nothrow_move_constructible<T>::value,
				"try_push(T&&) requires noexcept move construction.");

			unsigned spins = 0;
			for (;;)
			{
				const std::size_t pos = head_.load(std::memory_order_relaxed);
				Cell& cell = cells_[pos & mask_];
				const std::size_t seq = cell.sequence.load(std::memory_order_acquire);
				const intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos);
				if (diff == 0)
				{
					if (head_.compare_exchange_weak(
						pos,
						pos + 1,
						std::memory_order_relaxed,
						std::memory_order_relaxed))
					{
						::new (cell.storage) T(std::move(value));
						cell.sequence.store(pos + 1, std::memory_order_release);
						return true;
					}
				}
				else if (diff < 0)
					return false;

				detail::spin_backoff(spins);
			}
		}

		template <class... Args>
		[[nodiscard]] bool try_emplace(Args&&... args)
		{
			T tmp(std::forward<Args>(args)...);
			return try_push(std::move(tmp));
		}

		[[nodiscard]] bool try_pop(T& out) noexcept(std::is_nothrow_move_assignable<T>::value)
		{
			static_assert(
				std::is_nothrow_move_assignable<T>::value,
				"try_pop(T&) requires noexcept move assignment.");

			unsigned spins = 0;
			for (;;)
			{
				const std::size_t pos = tail_.load(std::memory_order_relaxed);
				Cell& cell = cells_[pos & mask_];
				const std::size_t seq = cell.sequence.load(std::memory_order_acquire);
				const intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos + 1);
				if (diff == 0)
				{
					if (tail_.compare_exchange_weak(
						pos,
						pos + 1,
						std::memory_order_relaxed,
						std::memory_order_relaxed))
					{
						T* const p = detail::launder_ptr<T>(cell.storage);
						out = std::move(*p);
						p->~T();
						cell.sequence.store(pos + capacity_, std::memory_order_release);
						return true;
					}
				}
				else if (diff < 0)
					return false;

				detail::spin_backoff(spins);
			}
		}

		/// 단일 스레드에서만 호출(셧다운 시 잔여 요소 수집).
		/// push_back 예외 시 해당 원소는 큐에서는 이미 제거된 뒤이므로, 사전 reserve 등으로 예외를 막는 것이 안전함.
		void drain_all(std::vector<T>& out)
		{
			static_assert(
				std::is_nothrow_move_constructible<T>::value,
				"drain_all uses try_pop_move_construct_at; same as try_push(T&&).");

			const std::size_t approx = size_approx();
			if (approx != 0)
				out.reserve(out.size() + approx);

			for (;;)
			{
				alignas(T) unsigned char raw[sizeof(T)];
				T* const p = detail::launder_ptr<T>(raw);
				if (!try_pop_move_construct_at(p))
					break;
				try
				{
					out.push_back(std::move(*p));
				}
				catch (...)
				{
					std::destroy_at(p);
					throw;
				}
				std::destroy_at(p);
			}
		}

	private:
		struct alignas(64) Cell
		{
			std::atomic<std::size_t> sequence{ 0 };
			alignas(T) unsigned char storage[sizeof(T)];

			Cell() noexcept = default;
		};

		/// 슬롯에 있는 T를 스택 버퍼로 옮긴 뒤 true(큐에서 제거됨). 비었으면 false.
		[[nodiscard]] bool try_pop_move_construct_at(void* dest_raw) noexcept(
			std::is_nothrow_move_constructible<T>::value)
		{
			static_assert(
				std::is_nothrow_move_constructible<T>::value,
				"try_pop_move_construct_at requires noexcept move construction.");

			unsigned char* const dest_bytes = static_cast<unsigned char*>(dest_raw);
			unsigned spins = 0;
			for (;;)
			{
				const std::size_t pos = tail_.load(std::memory_order_relaxed);
				Cell& cell = cells_[pos & mask_];
				const std::size_t seq = cell.sequence.load(std::memory_order_acquire);
				const intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos + 1);
				if (diff == 0)
				{
					if (tail_.compare_exchange_weak(
						pos,
						pos + 1,
						std::memory_order_relaxed,
						std::memory_order_relaxed))
					{
						T* const src = detail::launder_ptr<T>(cell.storage);
						::new (dest_bytes) T(std::move(*src));
						src->~T();
						cell.sequence.store(pos + capacity_, std::memory_order_release);
						return true;
					}
				}
				else if (diff < 0)
					return false;

				detail::spin_backoff(spins);
			}
		}

		void discard_all() noexcept
		{
			unsigned spins = 0;
			for (;;)
			{
				const std::size_t pos = tail_.load(std::memory_order_relaxed);
				Cell& cell = cells_[pos & mask_];
				const std::size_t seq = cell.sequence.load(std::memory_order_acquire);
				const intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos + 1);
				if (diff == 0)
				{
					if (tail_.compare_exchange_weak(
						pos,
						pos + 1,
						std::memory_order_relaxed,
						std::memory_order_relaxed))
					{
						T* const p = detail::launder_ptr<T>(cell.storage);
						p->~T();
						cell.sequence.store(pos + capacity_, std::memory_order_release);
						spins = 0;
						continue;
					}
				}
				else if (diff < 0)
					return;

				detail::spin_backoff(spins);
			}
		}

		std::size_t capacity_{};
		std::size_t mask_{};
		Cell* cells_{ nullptr };

		alignas(64) std::atomic<std::size_t> head_{ 0 };
		alignas(64) std::atomic<std::size_t> tail_{ 0 };
	};
} // namespace enf::lockfree
