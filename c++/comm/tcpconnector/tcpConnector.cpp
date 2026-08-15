#include "pch.h"
#include "tcpConnector.h"
namespace __ENF::nsconnector_v2
{
	tcpConnector::tcpConnector(const char* ip, int port, int connectiontimeoutMilisec/* = 3000*/)
		:m_socket(INVALID_SOCKET)
		, m_ip(ip)
		, m_port(port)
		, m_timeoutMs(connectiontimeoutMilisec)
		, m_closed(false)
	{

	}
	tcpConnector::~tcpConnector()
	{
		disconnect();
	}

	bool tcpConnector::connect()
	{
		SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (s == INVALID_SOCKET)
			return false;

		bool bIsBlocking = false;
		unsigned long blocking = bIsBlocking ? 0 : 1;
		ioctlsocket(s, FIONBIO, &blocking);
		setOption(s);

		SOCKADDR_IN stSocketAddr;
		memset(&stSocketAddr, NULL, sizeof(SOCKADDR_IN));
		stSocketAddr.sin_family = AF_INET;
		stSocketAddr.sin_port = htons(m_port); // htons : 리틀 엔디안 -> 빅 엔디안
		if (inet_pton(AF_INET, m_ip.c_str(), &stSocketAddr.sin_addr) != 1) // inet_addr 함수 대신
		{
			closesocket(s);
			return false;
		}

		if (::connect(s, (sockaddr*)&stSocketAddr, sizeof(stSocketAddr)) == SOCKET_ERROR)
		{
			const int nErr = WSAGetLastError();
			if (nErr != WSAEWOULDBLOCK)
			{
				shutdown(s, SD_BOTH);
				closesocket(s);
				s = INVALID_SOCKET;
				return false;
			}
		}

		if (waitsocketConnected(s, m_timeoutMs / 1000, m_timeoutMs % 1000))
		{
			bIsBlocking = true;
			blocking = bIsBlocking ? 0 : 1;
			ioctlsocket(s, FIONBIO, &blocking);

			std::lock_guard<std::mutex> g(m_closelock);
			m_closed.store(false);
			m_socket.store(s);
			return true;
		}
		
		shutdown(s, SD_BOTH);
		closesocket(s);
		return false;
	}

	void tcpConnector::shutdownconnection()
	{
		std::lock_guard<std::mutex> g(m_closelock);
		SOCKET s = m_socket.load();
		if (s != INVALID_SOCKET)
			shutdown(s, SD_BOTH);
		m_closed.store(true);
	}


	void tcpConnector::disconnect()
	{
		std::lock_guard<std::mutex> g(m_closelock);
		m_closed.store(true);
		SOCKET s = m_socket.exchange(INVALID_SOCKET);
		if (s != INVALID_SOCKET)
		{
			shutdown(s, SD_BOTH);
			closesocket(s);
		}
	}

	bool tcpConnector::isconnect()
	{
		return m_socket != INVALID_SOCKET && !m_closed.load();
	}

	bool tcpConnector::waitrecvsignal(int timeoutMs /*= 0*/)
	{
		const SOCKET s = m_socket.load();
		if (s == INVALID_SOCKET)
			return false;

		fd_set setR;
		FD_ZERO(&setR);
		FD_SET(s, &setR);

		timeval time_out = { 0 };
		time_out.tv_sec = timeoutMs / 1000;
		time_out.tv_usec = (timeoutMs % 1000) * 1000;

		// recv 가능하면 true 반환.
		return select(0, &setR, NULL, NULL, &time_out) == 1;
	}

	size_t tcpConnector::send(std::vector<unsigned char> vdata, int param/* = 0*/)
	{
		const SOCKET s = m_socket.load();
		size_t count = 0;
		if (s != INVALID_SOCKET && !m_closed.load())
		{
			while (count < vdata.size())
			{
				const int sent = ::send(s, (const char*)vdata.data() + count, (int)(vdata.size() - count), 0);
				if (sent == SOCKET_ERROR || sent == 0)
				{
					// 송수신만 중단.
					shutdownconnection();
					break;
				}
				count += static_cast<size_t>(sent);
			}
		}
		return count;
	}

	std::int64_t tcpConnector::recv(unsigned char* buffer, size_t buflen)
	{
		int nrecvSize = 0;
		const SOCKET s = m_socket.load();
		if (s == INVALID_SOCKET || m_closed.load())
			return -1;

		nrecvSize = ::recv(s, (char*)buffer, (int)buflen, 0);
		if (nrecvSize > 0) [[likely]]
			return nrecvSize;
		else if (nrecvSize == 0)
		{
			shutdownconnection();
			return -1;
		}

		const int err = WSAGetLastError();
		if (err == WSAETIMEDOUT || err == WSAEWOULDBLOCK)
			return 0;

		shutdownconnection();
		return -1;
	}

	bool tcpConnector::waitsocketConnected(const SOCKET& s, long ntimeoutsec, long ntimeoutMilisec)
	{
		fd_set setW, setE;

		FD_ZERO(&setW);
		FD_SET(s, &setW);
		FD_ZERO(&setE);
		FD_SET(s, &setE);

		timeval time_out = { 0 };
		time_out.tv_sec = ntimeoutsec;
		time_out.tv_usec = ntimeoutMilisec * 1000;

		int ret = select(0, NULL, &setW, &setE, &time_out);
		if (ret <= 0)
			return false; // select() failed or connection timed out

		if (FD_ISSET(s, &setE))
			return false; // connection failed

		return true;
	}

	void tcpConnector::setOption(SOCKET& s)
	{
		bool bKeepAlive = TRUE;
		if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char*)&bKeepAlive, sizeof(bKeepAlive)) == SOCKET_ERROR)
		{
			//PrintStdout("setsockopt : IPPROTO_TCP : SO_KEEPALIVE Error ! [code: %d]\n", WSAGetLastError());
		}

		/*SO_REUSEADDR :
		Allows a socket to bind to an address and port already in use.
		The SO_EXCLUSIVEADDRUSE option can prevent this.*/
		bool	bResuseAddr = TRUE;
		if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&bResuseAddr, sizeof(bResuseAddr)) == SOCKET_ERROR)
		{
			//PrintStdout("setsockopt : IPPROTO_TCP : SO_REUSEADDR Error ! [code: %d]\n", WSAGetLastError());
		}

		/*
		SO_LINGER : LINGER 구조체의 l_onoff 변수가 0이아닐 때 전송되어야 하는 큐의 남아있는 데이터를 전송하기 위해 closesocket 함수가 호출된 후에도
		소켓이 open된 상태로 유지된다. 열린 상태로 유지되는 시간은 l_linger의 변수에 따라 지정된다
		*/
		linger Ling = { 0, 0 };
		if (setsockopt(s, SOL_SOCKET, SO_LINGER, (char*)&Ling, sizeof(Ling)) == SOCKET_ERROR)
		{
			//PrintStdout("setsockopt : IPPROTO_TCP : SO_LINGER Error ! [code: %d]\n", WSAGetLastError());
		}

		/*
		SO_SNDBUF :
		The total per-socket buffer space reserved for sends.
		This is unrelated to SO_MAX_MSG_SIZE and does not necessarily correspond to the size of a TCP send window.*/
// 		if (setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char*)&MAX_SEND_BUF_SIZE, sizeof(MAX_SEND_BUF_SIZE)) == SOCKET_ERROR)
// 		{
// 			//PrintStdout("setsockopt : IPPROTO_TCP : SO_SNDBUF Error ! [code: %d]\n", WSAGetLastError());
// 			return FALSE;
// 		}
		/*
		SO_RCVBUF :
		The total per-socket buffer space reserved for receives.
		This is unrelated to SO_MAX_MSG_SIZE and does not necessarily correspond to the size of the TCP receive window.*/
// 		DWORD dwMaxBuf = MAX_RCV_BUF_SIZE;
// 		if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char*)&dwMaxBuf, sizeof(dwMaxBuf)) == SOCKET_ERROR)
// 		{
// 			//PrintStdout("setsockopt : IPPROTO_TCP : SO_RCVBUF Error ! [code: %d]\n", WSAGetLastError());
// 			return FALSE;
// 		}

		bool bUseNoDelay = true;
		// TCP_NODELAY : Nagel 알고리즘 사용 여부. TRUE로 설정시 사용하지 않지만 TCP 부하 증가, FALSE로 설정시 그 반대
		if (setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char*)&bUseNoDelay, sizeof(bUseNoDelay)) == SOCKET_ERROR)
		{
			//PrintStdout("setsockopt : IPPROTO_TCP : TCP_NODELAY Error ! [code: %d]\n", WSAGetLastError());
		}
	}


}