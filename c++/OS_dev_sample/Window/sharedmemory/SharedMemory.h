#pragma once
#include "IEnfConnector.h"
#include "EnfDllAPI.h"

/*
* boost 라이브러리를 사용한 공유 메모리.
공유 메모리 초기화시 메모리를 처음 생성하는 곳에서 Remove함수를 통하여 메모리 초기화 필요.
*/

namespace __ENF
{
	namespace CONNECTOR
	{
		class ENF_DLL_DEFINE __ENF::CONNECTOR::IEnfConnector;

		class ENF_DLL_DEFINE SharedMemory :
			public __ENF::CONNECTOR::IEnfConnector
		{
		public:
			// 송수신 버퍼 갯수, 사이즈 설정. BufferCount 0: 사용 안 함.
			SharedMemory();
			virtual ~SharedMemory();

			/* [Value] 기본 값: 0
			0: send packet 호출시 수신 메세지 큐 가득찬 경우 false 리턴. 
			1: send packet 호출시 수신 메세지 큐 가득찬 경우 전송 가능할 때 까지 block. 
			2: send packet 호출시 수신 메시지 큐 가득찬 경우 nTimeoutMilisec 동안 대기. 타임아웃시 false 리턴.
			*/
			void SetSendOption(int Value = 0, unsigned int nTimeoutMilisec = 500); 
			/* [Value] 기본 값: 2
			0: 메세지 큐에서 데이터 수신시 데이터 없는 경우 false 리턴.
			1: 메세지 큐에서 데이터 수신시 데이터 있을 때 까지 block. ( ** 해당 설정으로 운영중인 반대편 프로세스가 강제 종료될 경우 본 프로세스에서 send packet 호출시 block됌 ** )
			2: 메세지 큐에서 데이터 수신시 nTimeoutMilisec 동안 대기. 타임아웃시 false 리턴.
			*/
			void SetRecvOption(int Value = 0, unsigned int nTimeoutMilisec = 500);

			// 이미 존재하는 메모리 연결 시도시 boost::interprocess::interprocess_exception 발생.
			bool Connect(const ConnectionBase& Base) override;
			bool Disconnect() override;
			bool Reconnect() override; // fail: 접속 정보 없음
			bool IsConnectionEstablished() override;

			bool SendPacket(unsigned char* pBuffer, const size_t& nSize) override;
			bool SendPacket(std::vector<unsigned char>&& vBuffer) override;
			bool SendPacket(const std::vector<unsigned char>& vBuffer) override;
			bool ForceSendPacket(unsigned char* pBuffer, const size_t& nSize); // send option 1로 데이터 전송

			//remove memory from system.
			bool Remove();
			bool Remove(char* szIpcName);

			// get
			unsigned long GetLastError();

			const int& GetSendOption();
			const int& GetRecvOption();
			const unsigned int& GetSendBufferSize();
			const unsigned int& GetSendBufferCount();
			const unsigned int& GetRecvBufferSize();
			const unsigned int& GetRecvBufferCount();

		protected:
			void StartRecv();
			void StopRecv();
			bool SendMessage(void* pTarget, unsigned char* pBuffer, const size_t& nSize, unsigned int nPriority, int nSendOption);

		private:
			void RecvFunc();
			void notifyRecvThread();

			ConnectionBase m_CurBase;
			bool		 m_bIsTryingRecv;
			bool		 m_bIsTryConnect;
			int			 m_SendOption;
			int			 m_RecvOption;
			void*		 m_pMessageID;
			unsigned int m_nSendBufferCount;
			unsigned int m_nSendBufferSize;
			unsigned int m_nRecvBufferCount;
			unsigned int m_nRecvBufferSize;
			unsigned int m_nSendTimeout;
			unsigned int m_nRecvTimeout;

			void* m_ptRecv;
			void *m_pSender;
			void *m_pRecver;
			void* m_pReceiveAssist;

		};

	}
}