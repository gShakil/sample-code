#pragma once
#include "EventObserver.h"
#include <vector>

/*
Non - Thread Safe event / event observer 
*/

// 사용 예시
/*

class obs
	: public __ENF::EVENT::EventObserver<__ENF::DEVICE::EVT_TYPE, std::vector<char>>
{

public:
	void OnNotify(__ENF::DEVICE::EVT_TYPE evt, std::vector<char> vdata) override
	{
		std::cout << "obs: evt: " << (int)evt << ", data size: " << vdata.size() << std::endl;
	}

};

class obs2
	: public __ENF::EVENT::EventObserver<__ENF::DEVICE::EVT_TYPE, std::vector<char>>
{

public:
	void OnNotify(__ENF::DEVICE::EVT_TYPE evt, std::vector<char> vdata) override
	{
		std::cout << "obs2: evt: " << (int)evt << ", data size: " << vdata.size() << std::endl;
	}

};

class evt
	: public __ENF::EVENT::Event<__ENF::DEVICE::EVT_TYPE, std::vector<char>>
{
public:

};

int wmain(int argc, wchar_t* argv[])
{
	obs2 obs2;
	obs obs;
	evt evt;

	evt.AddObserver(&obs);
	evt.AddObserver(&obs2);

	std::vector<char> vEmpty;
	evt.EventNotify(__ENF::DEVICE::EVT_TYPE::EVT_DEVICE_CONNECTED, vEmpty);

	vEmpty.push_back(5);
	vEmpty.push_back(7);
	evt.EventNotify(__ENF::DEVICE::EVT_TYPE::EVT_DEVICE_DISCONNECTED, std::move(vEmpty));
	}
*/

namespace __ENF
{
	namespace EVENT
	{
		template<typename ...Args> class Event
		{
			using Observer = EventObserver<Args...>;
		public:
			void AddObserver(Observer* obs){
				m_vObserver.push_back(obs);
			}

			void RemoveObserver(Observer* obs) {
				auto find = std::find_if(m_vObserver.begin(), m_vObserver.end(),
					[obs](Observer* o)
					{
						return o == obs;
					}
				);
				if (find != m_vObserver.end())
					m_vObserver.erase(find);
			}

			template<typename ...Args>
			void EventNotify(Args... args) {
				for (auto& obs : m_vObserver)
					obs->OnEventNotify(args...);
			}

		protected:
			
		private:
			std::vector<Observer*> m_vObserver;
		};
	}
}