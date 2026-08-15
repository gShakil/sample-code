#pragma once
#include <type_traits>
/*
* std::any가 없는 c++17 이전에 std::any 대용으로 사용할 클래스.
* void* 사용시 불편을 줄여주자.
*/

namespace __ENF
{
	namespace TEMPLATE
	{
		namespace VER1_ALPHA
		{
			class voidDataBase { public: virtual ~voidDataBase() = default;	};
			template <typename T>
			class voidData
				: public voidDataBase
			{
			public:
				voidData() = default;                                      // 기본 생성자 (T가 기본 생성자를 지원하는 경우)
				explicit voidData(const T & data) : m_data(data) {}				 // 명시적 생성자
				voidData(T && data) noexcept : m_data(std::move(data)) {}  // 이동 생성자
				voidData(const voidData & other) : m_data(other.m_data) {}  // 복사 생성자
				voidData& operator=       (voidData && other) noexcept {						 // 이동 대입 연산자
					if (this != &other) {
						m_data = std::move(other.m_data);
					}
					return *this;
				}
				voidData& operator=       (const voidData& other) {						 // 복사 대입 연산자
					if (this != &other) {
						m_data = other.m_data;
					}
					return *this;
				}
				virtual ~voidData() = default;

				T& get() { return m_data; }
				const T& get() const { return m_data; }
				T* get_ptr() { return &m_data; }
				const T* get_ptr() const { return &m_data; }

				// ASSERT(intData.is_type<int>())
				template<typename U>
				bool is_type() const { return typeid(T) == typeid(U); }

				bool operator==(const voidData & other)  const { return m_data == other.m_data; }
				bool operator!=(const voidData & other)  const { return !(*this == other); }
				bool operator< (const voidData & other)  const { return m_data < other.m_data; }
				bool operator<=(const voidData & other)  const { return m_data <= other.m_data; }
				bool operator> (const voidData & other)  const { return m_data > other.m_data; }
				bool operator>=(const voidData & other)  const { return m_data >= other.m_data; }
			private:
				T m_data;
			};
		}
	}
}