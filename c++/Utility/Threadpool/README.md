## c++11 std threadpool

+ class ThreadPoolAssist
    + 스레드풀 기능 구현 클래스
+ class ThreadPoolUsage
    +  ThreadPoolAssist 클래스 사용 예제
    ```
    std::vector<std::future<int>> vRet;
	ThreadPoolAssist ass(nThreadCount);

    1. 
    for (int i = 0; i < (int)nRepeatCount; i++) 
    {
		futures.emplace_back(ass.EnqueueJob(work, i, i % 3 + 1));
	}
	for (auto& f : futures) {
		TRACE(" End - return id: %d\n", f.get());
	}

    2. 
    for (int i = 0; i < (int)nRepeatCount; i++) 
    {
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
    ```
