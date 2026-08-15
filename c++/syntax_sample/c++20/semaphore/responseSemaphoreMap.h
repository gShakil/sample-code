/*
__ENF::TEMPLATE::VER1_ALPHA::ResponseMap 을 c++20 을 사용해서 semaphore, std::any로 구현
*/

#pragma once
#include <atomic>
#include <semaphore>
#include <mutex>
#include <map>
#include <memory>
#include <any>
#include <chrono>

namespace __ENF::TEMPLATE::VER1_ALPHA
{
	enum class response_ret : int 
	{ 
		ALREADY_WAIT	 = -3,
		TIME_OUT         = -2, 
		NOT_EXIST        = -1, 
		STANDBY          = 0, 
		SUCC             = 1, 
		SUCC_LATE        = 2, 
	};
	struct response_body
	{
		response_ret ret{ response_ret ::STANDBY};
		long long elapse{ 0 };
	};

	class responseSemaphoreMap
	{
		struct Entry
		{
			Entry(uint64_t keyvalue)
				: keyvalue(keyvalue), semaphore(0) {}
			uint64_t				keyvalue;
			std::binary_semaphore	semaphore;
			std::atomic_bool		someone_complete{ false };
			std::atomic_bool		someone_wait{ false };
			std::any				data;
		};
		using pEntry = std::shared_ptr<Entry>;

	public:
		/*
		ClearALLEntry: 모든 응답 초기화, 삭제
		ResponseRecv의 호출 가능성이 있는 상태에서 호출 시 UB가 발생될 수 있음.
		초기화시에만 사용.
		*/
		void ClearALLEntry(); 
		/*
		대기 객체 생성.
		return true: 성공
		return false: key 값이 이미 존재.
		*/
		bool CreateEntry(uint64_t UniqueKey);

		/*
		대기 객체 삭제.
		객체 생성 후 WaitResponse를 호출하지 않는 경우 Entry가 삭제되지 않는다.
		결과와 무관하게 객체 삭제하는 함수.
		return true: 삭제 성공
		return false: 객체 없음, 삭제 실패
		*/
		bool RemoveEntry(uint64_t UniqueKey);

		/*
		응답 대기시 아래 함수 호출.
		반환되는 함수의 ret이 SUCC, SUCC_LATE인 경우에만 out_response 유효.
		응답 대기 결과와 무관하게 UniqueKey의 Entry는 삭제된다.
		*/
		response_body WaitResponse(uint64_t UniqueKey, int timeoutmilisec, std::any& out_response);

		// 응답 수신시 아래 함수 호출.
		bool ResponseRecv(uint64_t UniqueKey, std::any data);


	protected:

	private:
		std::mutex m_maplock;
		std::map<uint64_t, pEntry> m_responseMap;
	};

}



