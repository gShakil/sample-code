#pragma once
#include <functional>

namespace __ENF
{
	namespace EVENT
	{
		template<typename ...Args> class EventObserver
		{
		//using EvtCallback = std::function<void(Args&...)>;
		public:
			EventObserver(){

			}
			virtual ~EventObserver() {

			}

			virtual void OnEventNotify(Args... args) {};
		protected:

		private:

		};
	}
}


