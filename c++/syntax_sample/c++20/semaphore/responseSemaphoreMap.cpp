#include "pch.h"
#include "responseSemaphoreMap.h"
#include <vector>

namespace __ENF::TEMPLATE::VER1_ALPHA
{

	void responseSemaphoreMap::ClearALLEntry()
	{
		std::lock_guard<std::mutex> g(m_maplock);
		m_responseMap.clear();
	}

	bool responseSemaphoreMap::CreateEntry(uint64_t UniqueKey)
	{
		std::lock_guard<std::mutex> g(m_maplock);
		auto [it, succ] = m_responseMap.insert(
			{
			UniqueKey,
			std::make_shared<Entry>(UniqueKey)
			}
		);

		return succ;
	}

	bool responseSemaphoreMap::RemoveEntry(uint64_t UniqueKey)
	{
		std::lock_guard<std::mutex> g(m_maplock);
		if (m_responseMap.contains(UniqueKey))
		{
			m_responseMap.erase(UniqueKey);
			return true;
		}
		return false;
	}

	bool responseSemaphoreMap::ResponseRecv(uint64_t UniqueKey, std::any data)
	{
		pEntry ent;

		{
			std::lock_guard<std::mutex> g(m_maplock);
			auto find = m_responseMap.find(UniqueKey);
			if (find != m_responseMap.end())
				ent = find->second;
			else
			{
				// map에서 찾지 못했다면, 전혀 다른 UniqueKey가 수신되었거나,
				// 대기하는 부분에서 timeout으로 UniqueKey를 map에서 삭제한 경우가 해당된다.
				return false;
			}
		}

		bool expect = false;
		if (ent->someone_complete.compare_exchange_strong(expect, true))
		{
			// 응답 데이터 입력
			ent->data = std::move(data);
			ent->semaphore.release();
			return true;
		}
		else
		{
			// 응답이 수신되었지만, 대기하는 부분에서 이미 someone_complete를 true로 변경하여
			// 응답을 대기하고있지 않는 경우.
			return false;
		}
	}

	response_body responseSemaphoreMap::WaitResponse(uint64_t UniqueKey, int timeoutmilisec, std::any& out_response)
	{
		pEntry ent;
		response_body response;

		{
			std::lock_guard<std::mutex> g(m_maplock);
			auto find = m_responseMap.find(UniqueKey);
			if (find != m_responseMap.end())
				ent = find->second;
			else
			{
				response.ret = response_ret::NOT_EXIST;
				return response;
			}
		}

		bool ex = false;
		if (!ent->someone_wait.compare_exchange_strong(ex, true))
		{
			response.ret = response_ret::ALREADY_WAIT;
			return response;
		}

		/*
		semaphore 카운트가 0보다 클 때 감소 시도.
		return true: 감소 성공.
		return false: 감소 실패
		*/
		std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now();
		const bool getsignal = ent->semaphore.try_acquire_for(std::chrono::milliseconds(timeoutmilisec));
		response.elapse = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - tp).count();

		if (!getsignal)
		{
			/*
			timeout인 경우 진입.
			여기로 진입하는 도중 응답이 수신될 수 있다. 다시 한 번 체크하자.
			*/

			bool expect = false;
			if (ent->someone_complete.compare_exchange_strong(expect, true))
			{
				// 여기로 진입하면 타임아웃 확정.
				// 이 부분에서 someone_complete를 true로 변경했다.
				std::lock_guard<std::mutex> g(m_maplock);
				std::map<uint64_t, pEntry>::iterator it = m_responseMap.find(ent->keyvalue);
				if (it != m_responseMap.end() && it->second == ent)
					m_responseMap.erase(it);
				response.ret = response_ret::TIME_OUT;
				return response;
			}
			else
			{
				// someone_complete 변경에 실패하는 경우.
				// try_acquire_for 에 실패했지만 응답이 늦게나마 수신되어 수신하는 부분에서 someone_complete를 변경함.
				response.ret = response_ret::SUCC_LATE;
				ent->semaphore.acquire();
			}
		}
		else
			response.ret = response_ret::SUCC;

		// 응답 결과 체크하는 부분
		out_response = std::move(ent->data);

		{
			std::lock_guard<std::mutex> g(m_maplock);
			std::map<uint64_t, pEntry>::iterator it = m_responseMap.find(ent->keyvalue);
			if (it != m_responseMap.end() && it->second == ent)
				m_responseMap.erase(it);
		}

		return response;
	}

}