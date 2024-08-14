#include <iostream>
#include <thread>
#include <chrono>

using namespace std;
using namespace std::chrono_literals;

thread_local int id = 0;
void thread_func(int nid)
{
	while (true)
	{
		cout << "thread id: " << std::this_thread::get_id() << "id: " << id << endl;
		std::this_thread::sleep_for(2s);
	}


	// 	while (true)
	// 	{
	// 		console("thread id: 0x%x, id: %d", std::this_thread::get_id(), nid);
	// 		Sleep(500);
	// 	}
}

int main()
{
	std::thread t[10];

	for (int i = 0; i < 10; i++)
	{
		t[i] = std::thread(
			thread(thread_func, i)
		);
	}

	for (int i = 0; i < 10; i++)
		t[i].join();
}
