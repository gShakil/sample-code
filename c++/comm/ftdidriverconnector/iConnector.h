#pragma once
#include <vector>
#include <string>

namespace __ENF::nsconnector_v2
{
	class iConnector
	{
	public:
		virtual bool connect() = 0;
		virtual void disconnect() = 0;
		virtual bool isconnect() = 0;
		virtual bool waitrecvsignal() { return true; };

		/*
		param:
		- httpConnector: 
			0: get
			1: post
			2: put
		- other
			ignore
		*/
		virtual size_t send(std::vector<unsigned char> vdata, int param = 0) = 0;
		virtual std::int64_t recv(unsigned char* buffer, size_t buflen) = 0;
	protected:

	private:
	};
}