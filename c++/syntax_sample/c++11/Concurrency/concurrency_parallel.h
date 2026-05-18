	
	int _tmain()
	{
		std::vector<int> vInt;
		long long nAnswer = 0;
		for (int i = 1; i <= 10000; i++)
		{
			vInt.push_back(i);
			nAnswer += i;
		}
		
		int nSum = 0;
		std::mutex m;
		auto cur = TIME::GetCurrentTimePoint();
		concurrency::parallel_for_each(vInt.begin(), vInt.end(),
			[&nSum, &m](int n)
			{
				std::lock_guard<std::mutex> g(m);
				nSum += n;
				//TRACE(_T("n: %d, thread id: 0x%x\n"), n, GetCurrentThreadId());
			}
		);
		auto elapse = std::chrono::duration_cast<std::chrono::milliseconds>(TIME::GetElapsedTime(cur));
		console("#1 nSum: %d, nAnswer: %d. elapse: %lld", nSum, nAnswer, elapse.count());


		std::atomic<int> aSum = 0;
		cur = TIME::GetCurrentTimePoint();
		concurrency::parallel_for_each(vInt.begin(), vInt.end(),
			[&aSum](int n)
			{
				aSum += n;
				//TRACE(_T("n: %d, thread id: 0x%x\n"), n, GetCurrentThreadId());
			}
		);
		elapse = std::chrono::duration_cast<std::chrono::milliseconds>(TIME::GetElapsedTime(cur));
		console("#2 nSum: %d, nAnswer: %d. elapse: %lld", (int)aSum, nAnswer, elapse.count());

		
		concurrency::combinable<int> cSum;
		cur = TIME::GetCurrentTimePoint();
		concurrency::parallel_for_each(vInt.begin(), vInt.end(),
			[&cSum](int n)
			{
				cSum.local() += n;
				//TRACE(_T("n: %d, thread id: 0x%x\n"), n, GetCurrentThreadId());
			}
		);

		elapse = std::chrono::duration_cast<std::chrono::milliseconds>(TIME::GetElapsedTime(cur));
		console("#3 nSum: %d, nAnswer: %d. elapse: %lld", cSum.combine(std::plus<int>()), nAnswer, elapse.count());
	}