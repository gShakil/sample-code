#pragma once
#include "IEnfConnector.h"
#include "EnfDllAPI.h"

/*
CSharedMemory.h를 사용하여 send, recv 양방향 통신을 하는 공유 메모리 클래스.
*/

namespace __ENF
{
	namespace CONNECTOR
	{
		class ENF_DLL_DEFINE __ENF::CONNECTOR::IEnfConnector;

		class ENF_DLL_DEFINE WinAPISharedMemory :
			public __ENF::CONNECTOR::IEnfConnector
		{

		public:
			WinAPISharedMemory();
			virtual ~WinAPISharedMemory();

			/*
			ConnectionBase:: nSendBufferCount, nRecvBufferCount 사용 안 함
			*/
			bool Connect                (const ConnectionBase& Base)                  override;
			bool Disconnect             ()                                            override;
			bool Reconnect              ()                                            override; // fail: 접속 정보 없음
			bool IsConnectionEstablished()                                            override;

			// Auto Lock. write header + data
			bool SendPacket             (unsigned char* pBuffer, const size_t& nSize) override;
			bool SendPacket             (std::vector<unsigned char>&& vBuffer)        override;
			bool SendPacket             (const std::vector<unsigned char>& vBuffer)   override;
			
			bool Lock();
			bool SendPacketNoLock(unsigned char* pBuffer, const size_t& nSize, const size_t& totSize); // Manual Lock. write header + data. totSize: AppendPacketNoLock를 통해서도 전송되는 모든 데이터의 합 입력.
			bool AppendPacketNoLock(unsigned char* pBuffer, const size_t& nSize); // Manual Lock. write data
			bool Unlock();

			// Close Memory. Reset Shared Memory
			bool Remove();
		protected:
			//bool sendPakcet(unsigned char* headerbuf, unsigned char* buf, const size_t& size);
			bool sendPakcet(unsigned char* buf, const size_t& size);
			void StartRecv();
			void StopRecv();

		private:
			void AllocMemory(const ConnectionBase& Base);
			void FreeMemory();

			bool OpenMemory(void* mem, const char* name);
			ConnectionBase m_CurBase;
			void RecvFunc();
			bool m_bIsTryingRecv;
			bool m_bIsCon;

			void* m_sender;
			void* m_recver;
			void* m_ptRecv;
		};

	}
}