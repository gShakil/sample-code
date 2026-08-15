#pragma once
#include "iConnector.h"
#include <string>
#include <atomic>

namespace __ENF::nsconnector_v2
{
	class serialConnector :
		public iConnector
	{
	public:
		/*
		Baudrate 115,200
		Databit 8				Number of bits/byte, 4-8
		Stopbits 0				0,1,2 = 1, 1.5, 2
		Parity 0				0-4 = None,Odd,Even,Mark,Space
		szPortName: COM1, COM2, COM3, ... COM10
		*/
		serialConnector(const char* szPortName, DWORD dwBaud = 115200, BYTE byData = 8, BYTE byStop = 0, BYTE byParity = 0);
		virtual ~serialConnector();

		bool connect() override;
		void disconnect() override;
		bool isconnect() override;
		bool waitrecvsignal() override;

		size_t			send(std::vector<unsigned char> vdata, int param = 0) override;
		std::int64_t	recv(unsigned char* buffer, size_t buflen) override;

	protected:

	private:
		std::string m_port;
		std::atomic_bool m_bisconnect;
		DWORD m_baudrate;
		BYTE m_databit;
		BYTE m_stopbit;
		BYTE m_parity;


		HANDLE		m_hComm;				// 통신 포트 파일 핸들
		OVERLAPPED	m_osRead, m_osWrite;	// 포트 파일 Overlapped structure

	};
}
