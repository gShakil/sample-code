#include "pch.h"
#include "ThreadPoolUsage.h"
#include "ThreadPoolAssist.h"
#include <random>
#include <vector>

using __ENF::Threadpool::ThreadPoolAssist;

int work(int nId, int nsec) {
	std::this_thread::sleep_for(std::chrono::seconds(nsec % 5));
	TRACE(_T("[Thread ID] 0x%02x, [Argument] id: %d, nsec: %d return : %d\n")
		, std::this_thread::get_id()
		, nId
		, nsec
		, nsec
	);
	return nId;
}

int ThreadPoolUsage::WorkFunction(int nId, int nSec)
{
	std::this_thread::sleep_for(std::chrono::seconds(nSec % 5));
	TRACE(_T("[Thread ID] 0x%02x, [Argument] id: %d, nsec: %d return : %d\n")
		, std::this_thread::get_id()
		, nId
		, nSec
		, nSec
	);
	return nId;
}
void ThreadPoolUsage::SampleExec_Type1(const size_t& nThreadCount, const size_t& nRepeatCount)
{
	ThreadPoolAssist ass(nThreadCount);

	std::vector<std::future<int>> futures;
	for (int i = 0; i < (int)nRepeatCount; i++) {
		futures.emplace_back(ass.EnqueueJob(work, i, i % 3 + 1));
	}

	for (auto& f : futures) {
		TRACE(" End - return id: %d\n", f.get());
	}
}

void ThreadPoolUsage::SampleExec_Type2(const size_t& nThreadCount, const size_t& nRepeatCount)
{
	auto pMakeDummy =
		[&](int nId, int nsec) -> int
	{
		std::this_thread::sleep_for(std::chrono::seconds(nsec % 5));
		TRACE(_T("[Thread ID] 0x%02x, [Argument] id: %d, nsec: %d return : %d\n")
			, std::this_thread::get_id()
			, nId
			, nsec
			, nsec
		);
		return nId;
	};

	std::vector<std::future<int>> vRet;
	ThreadPoolAssist ass(nThreadCount);

	for (size_t i = 0; i < nRepeatCount; i++)
	{
		vRet.emplace_back(
			ass.EnqueueJob(
				pMakeDummy, i, i % 3 + 1
			)
		);
	}

	for (auto& f : vRet) {
		TRACE("Thread End - return: %d \n", f.get());
	}
}

void ThreadPoolUsage::SampleExec_Type3(const size_t& nThreadCount, const size_t& nRepeatCount)
{
	ThreadPoolAssist ass(nThreadCount);

	std::vector<std::future<int>> futures;
	for (int i = 0; i < (int)nRepeatCount; i++) {
		futures.emplace_back(ass.EnqueueJob(
			std::bind(&ThreadPoolUsage::WorkFunction, this, std::placeholders::_1, std::placeholders::_2),
			i,
			i % 3 + 1
		)
		);
	}

	for (auto& f : futures) {
		TRACE(" End - return id: %d\n", f.get());
	}
}

