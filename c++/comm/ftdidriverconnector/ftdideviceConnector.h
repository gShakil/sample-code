#pragma once
#include <vector>
#include <string>
#include <atomic>
#include "iConnector.h"

namespace __ENF::nsconnector_v2
{
	class ftdideviceConnector :
		public iConnector
	{
	public:
		/*
		byData: 7 or 8(default). 7: FT_BITS_7, 8: FT_BITS_8
		byStop: 1(default) or 2. 1: FT_STOP_BITS_1, 2: FT_STOP_BITS_2
		byParity: 0(default) ~ 4. 0: FT_PARITY_NONE, 1: FT_PARITY_ODD, 2: FT_PARITY_EVEN, 3: FT_PARITY_MARK, 4: FT_PARITY_SPACE
		*/

		ftdideviceConnector         (const char* szPortName, DWORD dwBaud = 115200, BYTE byData = 8, BYTE byStop = 1, BYTE byParity = 0);
		virtual ~ftdideviceConnector();

		void setprintdebug          (bool value); // default: false

		bool connect                () override;
		void disconnect             () override;
		bool isconnect              () override;
		bool waitrecvsignal			() override;

		size_t			send(std::vector<unsigned char> vdata, int param = 0) override;
		std::int64_t	recv(unsigned char* buffer, size_t buflen) override;
	protected:

	private:
		void printdebug             (std::string_view vs);
		std::string vec2string      (std::vector<unsigned char> vdat);

		void* m_deviceHandle;
		std::string m_port;
		std::atomic_bool m_bisconnect;
		bool m_bprintdebug;
		HANDLE m_recvEvent;
		DWORD m_baudrate;
		BYTE m_databit;
		BYTE m_stopbit;
		BYTE m_parity;
	};
}

