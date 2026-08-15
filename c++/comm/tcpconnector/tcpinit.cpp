#include "pch.h"
#include "tcpinit.h"
#include <ws2tcpip.h> // inet_pton

tcpInit::tcpInit()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) == SOCKET_ERROR)
	{
		return;
	}
}

tcpInit::~tcpInit()
{
	if (WSACleanup() == SOCKET_ERROR)
	{
	}
}