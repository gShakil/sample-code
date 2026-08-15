#pragma once
#include <functional> // std::function
#include <vector>
#include "EnfDllAPI.h"

/*
TCP, Serial, 기타 방식의 연결 인터페이스
*/

namespace __ENF
{
	namespace CONNECTOR
	{
// 		template class ENF_DLL_DEFINE std::function<void()>;
// 		template class ENF_DLL_DEFINE std::function<unsigned int(unsigned char* pBuffer, const unsigned int& nSize)>;

		union /*ENF_DLL_DEFINE*/ ConnectionBase
		{
// 			ConnectionBase() 
// 			{
// 				memset(this, 0, sizeof(ConnectionBase));
// 			};
			struct TCP_BASE
			{
				char szIP[32];
				unsigned int uPort;
			} tcp;
			struct SERIAL_BASE
			{
				char szComport[32];
				unsigned int nBaudrate;
				unsigned char nNumofBits;
				unsigned char nStopbits;
				unsigned char nParity;
			} serial;
			struct SHAREDMEM_BASE
			{
				char szSendIpcName[128];
				char szRecvIpcName[128];
				//BufferCount 0: 사용 안 함.
				unsigned int nSendBufferByteSize;
				unsigned int nSendBufferCount;
				unsigned int nRecvBufferByteSize;
				unsigned int nRecvBufferCount;
			} share_mem;
		};

		class IEnfConnector
		{
		public:
			virtual bool Connect(const ConnectionBase& Base) = 0;
			virtual bool Disconnect()				 = 0;
			virtual bool Reconnect()				 = 0;
			virtual bool IsConnectionEstablished()	 = 0;

			virtual bool SendPacket(unsigned char* pBuffer, const size_t& nSize)	= 0;
			virtual bool SendPacket(std::vector<unsigned char>&& vBuffer)			= 0;
			virtual bool SendPacket(const std::vector<unsigned char>& vBuffer)		= 0;

			// SetPacketReceiveCallback: return: 처리된 패킷 갯수 반환.

			void SetPacketReceiveCallback	(std::function<unsigned int(unsigned char* pBuffer, const unsigned int& nSize)> pFunc) { m_pPacketReceiveCallback = pFunc;}
			void SetOnConnectCallback		(std::function<void()> pFunc) { m_pOnConnectCallback = pFunc; };
			void SetOnDisConnectCallback	(std::function<void()> pFunc) { m_pOnDisconnectCallback = pFunc; };

		protected:
			std::function<unsigned int(unsigned char*, const unsigned int&)> m_pPacketReceiveCallback;
			std::function<void()> m_pOnConnectCallback;
			std::function<void()> m_pOnDisconnectCallback;
		};

	}
}

