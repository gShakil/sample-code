#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>
#include <utility>

/**
 * Fixed-size SPSC FIFO lock-free bounded ring buffer.
 * - Capacity(N)은 반드시 2의 거듭제곱이어야 인덱스 마스킹이 작동함.
 */
template <typename T, std::size_t N>
class StaticSPSCLockFreeQueue
{
    static_assert(N > 0 && (N & (N - 1)) == 0, "Capacity N must be a power of 2.");
    static_assert(std::is_nothrow_destructible<T>::value, "T must be nothrow destructible.");

private:
    struct alignas(64) Cell
    {
        alignas(T) unsigned char storage[sizeof(T)];
    };

    // 멤버 변수 순서 및 alignas는 캐시 라인 간섭(False Sharing) 방지를 위함
    const std::size_t m_mask = N - 1;

    alignas(64) std::atomic<std::uint64_t> m_readIdx{ 0 };
    alignas(64) std::atomic<std::uint64_t> m_writeIdx{ 0 };

    // 정해진 갯수만큼 미리 할당된 저장소
    Cell m_cells[N];

    static T* launder_ptr(void* p) noexcept {
#if defined(__cpp_lib_launder) && __cpp_lib_launder >= 201606L
        return std::launder(reinterpret_cast<T*>(p));
#else
        return reinterpret_cast<T*>(p);
#endif
    }

public:
    StaticSPSCLockFreeQueue() = default;

    // 복사 및 이동 제한 (정적 배열을 포함하므로 무거움)
    StaticSPSCLockFreeQueue(const StaticSPSCLockFreeQueue&) = delete;
    StaticSPSCLockFreeQueue& operator=(const StaticSPSCLockFreeQueue&) = delete;

    ~StaticSPSCLockFreeQueue() noexcept {
        T dummy;
        while (try_pop(dummy)) {
            // 모든 남은 원소 소멸
        }
    }

    constexpr std::size_t capacity() const noexcept { return N; }

    bool isEmpty() const noexcept {
        return m_readIdx.load(std::memory_order_relaxed) == m_writeIdx.load(std::memory_order_relaxed);
    }

    bool isFull() const noexcept {
        const std::uint64_t w = m_writeIdx.load(std::memory_order_relaxed);
        const std::uint64_t r = m_readIdx.load(std::memory_order_acquire);
        return (w - r) >= N;
    }

    [[nodiscard]] bool try_push(const T& value) {
        return try_emplace(value);
    }

    [[nodiscard]] bool try_push(T&& value) noexcept(std::is_nothrow_move_constructible<T>::value) {
        return try_emplace(std::move(value));
    }

    template <class... Args>
    [[nodiscard]] bool try_emplace(Args&&... args) {
        const std::uint64_t w = m_writeIdx.load(std::memory_order_relaxed);
        const std::uint64_t r = m_readIdx.load(std::memory_order_acquire);

        if (w - r >= N) return false;

        Cell& cell = m_cells[w & m_mask];
        ::new (cell.storage) T(std::forward<Args>(args)...);

        m_writeIdx.store(w + 1, std::memory_order_release);
        return true;
    }
    /**
     * 데이터를 쓸 공간을 미리 확보하여 포인터로 반환합니다.
     * 반환된 포인터 위치에 직접 데이터를 채워넣은 후, 반드시 commit_push를 호출해야 합니다.
     */
    [[nodiscard]] T* try_enqueue_prepare() noexcept {
        const std::uint64_t w = m_writeIdx.load(std::memory_order_relaxed);
        const std::uint64_t r = m_readIdx.load(std::memory_order_acquire);

        if (w - r >= N) return nullptr; // 큐가 가득 참

        Cell& cell = m_cells[w & m_mask];
        // 아직 T 객체가 생성되지 않은 Raw 메모리 주소를 반환
        return reinterpret_cast<T*>(cell.storage);
    }

    /**
     * 데이터를 다 채웠음을 알리고 consumer가 볼 수 있게 index를 넘깁니다.
     * constructor_needed: T가 단순 구조체(POD)가 아니라서 생성자 호출이 필요한 경우 true
     */
    void commit_push() noexcept {
        const std::uint64_t w = m_writeIdx.load(std::memory_order_relaxed);
        // index를 증가시켜 소비자에게 공개
        m_writeIdx.store(w + 1, std::memory_order_release);
    }

    [[nodiscard]] bool try_pop(T& out) noexcept(std::is_nothrow_move_assignable<T>::value) {
        const std::uint64_t r = m_readIdx.load(std::memory_order_relaxed);
        const std::uint64_t w = m_writeIdx.load(std::memory_order_acquire);

        if (r >= w) return false;

        Cell& cell = m_cells[r & m_mask];
        T* const p = launder_ptr(cell.storage);

        out = std::move(*p);
        p->~T();

        m_readIdx.store(r + 1, std::memory_order_release);
        return true;
    }

	[[nodiscard]] T* try_pop_prepare() noexcept {
		const std::uint64_t r = m_readIdx.load(std::memory_order_relaxed);
		const std::uint64_t w = m_writeIdx.load(std::memory_order_acquire);

		if (r >= w) return nullptr;

		Cell& cell = m_cells[r & m_mask];
		return launder_ptr(cell.storage);
	}

	void commit_pop() noexcept {
        const std::uint64_t r = m_readIdx.load(std::memory_order_relaxed);
        Cell& cell = m_cells[r & m_mask];

        launder_ptr(cell.storage)->~T();

		// index를 증가시켜 생산자에게 공개
        m_readIdx.store(r + 1, std::memory_order_release);
	}
};