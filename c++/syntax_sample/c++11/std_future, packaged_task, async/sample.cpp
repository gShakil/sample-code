#include <iostream>
#include <thread>
#include <chrono>



int main()
{
/*
	using return_value = int;
	using func_param1 = int;

	//////////////////////////////////////////////////////////////////////////
	// 1. using std::future
	//////////////////////////////////////////////////////////////////////////
	func_param1 param1 = 134;
	std::promise<return_value> promise;
	std::future< return_value> future = promise.get_future();
	std::thread thread(
		[&](std::promise<return_value>* p, func_param1 param1)
		{
			return_value value = param1;
			p->set_value(value);

		}, 
		&promise,
		param1
	);

	future.wait();
	return_value ret1 = future.get();
	thread.join();


	//////////////////////////////////////////////////////////////////////////
	// 2. using std::packaged_task
	//////////////////////////////////////////////////////////////////////////
	// return_value 리턴, 함수 인자: func_param1
	std::packaged_task<return_value(func_param1)> task(
		[&](func_param1 param)
		{
			return_value ret = param;
			return ret;
		}
	);
	std::future< return_value> future2 = task.get_future();
	func_param1 param2 = 55;
	std::thread t(std::move(task), param2);
	return_value ret2 = future2.get();
	t.join();

	//////////////////////////////////////////////////////////////////////////
	// 3. using std::async
	//////////////////////////////////////////////////////////////////////////
	func_param1 param3 = 999;
	std::future< return_value> future3 = std::async(
		std::launch::async, // 바로 스레드 생성
		[&](func_param1 param)
		{
			return_value ret = param;

			return ret;
		},
		param3
	);
	return_value ret3  = future3.get();
*/
}
