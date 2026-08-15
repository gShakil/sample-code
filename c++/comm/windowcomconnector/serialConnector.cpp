#include "pch.h"
#include "serialConnector.h"

namespace __ENF::nsconnector_v2
{
	serialConnector::serialConnector(const char* szPortName, DWORD dwBaud, BYTE byData, BYTE byStop, BYTE byParity)
		:m_port(szPortName)
		, m_baudrate(dwBaud)
		, m_databit(byData)
		, m_stopbit(byStop)
		, m_parity(byParity)
		, m_hComm(INVALID_HANDLE_VALUE)
	{
		ZeroMemory(&m_osRead, sizeof(m_osRead));
		ZeroMemory(&m_osWrite, sizeof(m_osWrite));
	}

	serialConnector::~serialConnector()
	{

	}

	bool serialConnector::connect()
	{
		COMMTIMEOUTS	timeouts;
		DCB				dcb;

		m_osRead.Offset = 0;
		m_osRead.OffsetHigh = 0;
		if (!(m_osRead.hEvent = CreateEventA(NULL, TRUE, FALSE, NULL)))
			return false;


		m_osWrite.Offset = 0;
		m_osWrite.OffsetHigh = 0;
		if (!(m_osWrite.hEvent = CreateEventA(NULL, TRUE, FALSE, NULL)))
			return false;

		char szPort[32] = { NULL, };
		sprintf_s(szPort, "\\\\.\\%s", m_port.c_str());

		m_hComm = CreateFileA(szPort,
			GENERIC_READ | GENERIC_WRITE, 0, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
			NULL);

		if (m_hComm == (HANDLE)-1)
			return false;


		// 데이터가 들어오면.. 수신 이벤트가 발생
		if (!SetCommMask(m_hComm, EV_RXCHAR)) 
		{
		}

		// InQueue, OutQueue 크기 설정.
		if (!SetupComm(m_hComm, 1024, 1024)) 
		{
		}

		// 포트 비우기.
		if (!PurgeComm(m_hComm,
			PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR)) 
		{
		}

		// timeout 설정.
		timeouts.ReadIntervalTimeout = 0xFFFFFFFF;
		timeouts.ReadTotalTimeoutMultiplier = 0;
		timeouts.ReadTotalTimeoutConstant = 0;

		timeouts.WriteTotalTimeoutMultiplier = 2 * CBR_9600 / m_baudrate;
		timeouts.WriteTotalTimeoutConstant = 0;

		if (!SetCommTimeouts(m_hComm, &timeouts)) {
			//trace_out("%s :: SetCommTimeouts Fail. Err: %d", __FUNCTION__, GetLastError());
		}

		dcb.DCBlength = sizeof(DCB);
		if (!GetCommState(m_hComm, &dcb)) 
		{
		}

		dcb.BaudRate = m_baudrate;
		dcb.ByteSize = m_databit;
		dcb.Parity = m_parity;
		dcb.StopBits = m_stopbit;


		if (!SetCommState(m_hComm, &dcb))
			return false;

		m_bisconnect = true;
		return true;
	}

	void serialConnector::disconnect()
	{
		BOOL bResult = FALSE;

		//--> 마스크 해제..
		bResult = SetCommMask(m_hComm, 0);
		if (!bResult) {
		}

		//--> 포트 비우기.
		bResult = PurgeComm(m_hComm, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR);
		if (!bResult) {
		}

		bResult = CloseHandle(m_hComm);
		if (!bResult) {
		}

		m_bisconnect = false;
	}

	size_t serialConnector::send(std::vector<unsigned char> vdata, int param/* = 0*/)
	{
		DWORD	dwWritten, dwError, dwErrorFlags;
		COMSTAT	comstat;

		if (m_bisconnect == false)
			return false;

		if (!WriteFile(m_hComm, vdata.data(), (DWORD)vdata.size(), &dwWritten, &m_osWrite))
		{
			if (GetLastError() == ERROR_IO_PENDING)
			{
				// 읽을 문자가 남아 있거나 전송할 문자가 남아 있을 경우 Overapped IO의
				// 특성에 따라 ERROR_IO_PENDING 에러 메시지가 전달된다.
				//timeouts에 정해준 시간만큼 기다려준다.
				while (!GetOverlappedResult(m_hComm, &m_osWrite, &dwWritten, TRUE))
				{
					dwError = GetLastError();
					if (dwError != ERROR_IO_INCOMPLETE)
					{
						ClearCommError(m_hComm, &dwErrorFlags, &comstat);
						break;
					}
				}
			}
			else
			{
				dwWritten = 0;
				ClearCommError(m_hComm, &dwErrorFlags, &comstat);
			}
		}

		return dwWritten;
	}
	bool serialConnector::isconnect()
	{
		return m_bisconnect;
	}

	bool serialConnector::waitrecvsignal()
	{
		if (!isconnect())
			return false;

		// 데이터 수신될 때 까지 대기.
		DWORD dwEvent = 0;
		WaitCommEvent(m_hComm, &dwEvent, NULL);
		if ((dwEvent & EV_RXCHAR) == EV_RXCHAR)
			return true;

		return false;
	}

	std::int64_t serialConnector::recv(unsigned char* buffer, size_t buflen)
	{
		DWORD	dwRead, dwError, dwErrorFlags;
		COMSTAT comstat;

		//--- system queue에 도착한 byte수만 미리 읽는다.
		ClearCommError(m_hComm, &dwErrorFlags, &comstat);

		//--> 시스템 큐에서 읽을 거리가 있으면..
		dwRead = comstat.cbInQue;
		if (dwRead > 0)
		{
			//--> 버퍼에 일단 읽어들이는데.. 만일..읽어들인값이 없다면..
			if (!ReadFile(m_hComm, buffer, (DWORD)buflen, &dwRead, &m_osRead))
			{
				//--> 읽을 거리가 남았으면..
				if (GetLastError() == ERROR_IO_PENDING)
				{
					//--------- timeouts에 정해준 시간만큼 기다려준다.
					while (!GetOverlappedResult(m_hComm, &m_osRead, &dwRead, TRUE))
					{
						dwError = GetLastError();
						if (dwError != ERROR_IO_INCOMPLETE)
						{
							ClearCommError(m_hComm, &dwErrorFlags, &comstat);
							break;
						}
					}
				}
				else
				{
					dwRead = 0;
					ClearCommError(m_hComm, &dwErrorFlags, &comstat);
				}
			}
		}

		return dwRead;
	}
}