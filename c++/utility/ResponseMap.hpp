#pragma once
#include <mutex>
#include <map>
#include "voidData.hpp"


namespace __ENF
{
	namespace TEMPLATE
	{
		namespace VER1_ALPHA
		{
			struct ResponseCond
			{
				std::unique_ptr<__ENF::TEMPLATE::VER1_ALPHA::voidDataBase> responseData;
				std::condition_variable cond;
				std::mutex mutex;
			};

			class ResponseMap
			{
			public:
				void clear()
				{
					std::lock_guard<std::mutex> g(m_lock);
					m_mcond.clear();
				}
				template <typename T>
				bool wait_for_response(unsigned long long key, unsigned long long milisec, T& outData, unsigned long long* poutElapseMilisec = nullptr)
				{
					{
						std::lock_guard<std::mutex> g(m_lock);
						std::unique_ptr< ResponseCond> p = std::make_unique<ResponseCond>();
						m_mcond[key] = std::move(p);
					}

					std::chrono::steady_clock::time_point np;
					if (poutElapseMilisec)
						np = std::chrono::steady_clock::now();

					std::unique_lock<std::mutex> ul(m_mcond[key]->mutex);
					// 시간이 경과했을 때 혹은 notify로 깨어났을 때 wait_for에 입력된 pred(여기선 람다)조건을 판별.
					// 결과값으로 리턴값(여기선 wait_succ)이 정해진다.
					const bool wait_succ = m_mcond[key]->cond.wait_for(
						ul, std::chrono::milliseconds(milisec),
						[this, &key] {
							return m_mcond[key]->responseData.get() != nullptr;
						}
					);

					ul.unlock();
					{
						std::lock_guard<std::mutex> g(m_lock);
						if (wait_succ)
						{
							using namespace __ENF::TEMPLATE::VER1_ALPHA;

							std::unique_ptr<voidData<T>> pData(dynamic_cast<voidData<T>*>(m_mcond[key]->responseData.release()));
							outData = pData->get();
						}
						m_mcond.erase(key);
					}
					if (poutElapseMilisec)
					{
						auto mili = std::chrono::duration_cast<std::chrono::milliseconds>(
							std::chrono::steady_clock::now() - np);
						*poutElapseMilisec = mili.count();
					}
					return wait_succ;
				}
				template <typename T>
				bool response_receive(unsigned long long key, const T& receivedData)
				{
					std::lock_guard<std::mutex> g(m_lock);
					auto find = m_mcond.find(key);
					if (find == m_mcond.end())
						return false; // unexpected

					using vdata = __ENF::TEMPLATE::VER1_ALPHA::voidData<T>;
					std::unique_lock<std::mutex> ul(m_mcond[key]->mutex);
					m_mcond[key]->responseData = std::make_unique< vdata>(receivedData);
					m_mcond[key]->cond.notify_one();
					return true;
				}
				bool iswaitable(unsigned long long key)
				{
					std::lock_guard<std::mutex> g(m_lock);
					auto find = m_mcond.find(key);
					return (find != m_mcond.end());
				}
			protected:

			private:
				std::mutex m_lock;
				std::map<unsigned long long, std::unique_ptr<ResponseCond>> m_mcond;
			};
		}
	}
}