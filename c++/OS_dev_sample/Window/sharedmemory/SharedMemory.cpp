#include "pch.h"
#include "SharedMemory.h"
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <thread>
#include <chrono>
#include <map>
#include <vector>
#include <boost/interprocess/detail/win32_api.hpp>
using namespace boost::interprocess;

#define _SENDER ((message_queue*)m_pSender)
#define _RECVER ((message_queue*)m_pRecver)
#define CONV_QUEUE(x) ((message_queue*)x)
#define _RECV_THREAD ((std::thread*)m_ptRecv)
#define _RECV_ASSIST ((std::map<unsigned int, std::vector<MSG_QUEUE_STRUCT_INFO_ARRAY>>*)m_pReceiveAssist)
#define _MESSAGE_ID ((std::atomic<int>*)m_pMessageID)

struct MSG_QUEUE_STRUCT_INFO
{
	unsigned int nZeroBaseMessageID; // 메세지 고유 ID. 새로운 메세지가 전송될 때 1씩 증가.
	unsigned int nZeroBaseCurMessageIndex; // 쪼개어 보내질 경우의 메세지 인덱스
	unsigned int nZeroBaseMaxMessageIndex; // 쪼개어 보내질 경우 마지막 메세지 인덱스
	std::chrono::steady_clock::time_point tp; // 메세지 전송 시각.
	// 구조체 뒤에 실제 데이터 추가
};
struct MSG_QUEUE_STRUCT_INFO_ARRAY
{
	MSG_QUEUE_STRUCT_INFO stmsg;
	std::vector<unsigned char> vPacket;
};

__ENF::CONNECTOR::SharedMemory::SharedMemory()
	:m_bIsTryConnect(false),
	m_SendOption(0),
	m_nSendTimeout(500),
	m_RecvOption(0),
	m_nRecvTimeout(100),
	m_pSender(nullptr),
	m_pRecver(nullptr),
	m_ptRecv(nullptr),
	m_bIsTryingRecv(false)
{
	m_pMessageID = new std::atomic<int>;
	m_pReceiveAssist = (void*) new std::map<unsigned int, std::vector<MSG_QUEUE_STRUCT_INFO_ARRAY>>;
	ZeroMemory(&m_CurBase, sizeof(m_CurBase));
}

__ENF::CONNECTOR::SharedMemory::~SharedMemory()
{
	Disconnect();
	if (_MESSAGE_ID)
	{
		delete _MESSAGE_ID;
		m_pMessageID = NULL;
	}
	if (_RECV_ASSIST)
	{
		delete _RECV_ASSIST;
		m_pReceiveAssist = nullptr;
	}

}

void __ENF::CONNECTOR::SharedMemory::SetSendOption(int Value /* = 0 */, unsigned int nTimeoutMilisec /* = 500 */)
{
	m_SendOption = Value;
	m_nSendTimeout = nTimeoutMilisec;
}

void __ENF::CONNECTOR::SharedMemory::SetRecvOption(int Value /*= 1*/, unsigned int nTimeoutMilisec /*= 500*/)
{
	m_RecvOption = Value;
	m_nRecvTimeout = nTimeoutMilisec;
	notifyRecvThread();
}

bool __ENF::CONNECTOR::SharedMemory::Connect(const ConnectionBase& Base)
{
	try
	{
		m_nSendBufferCount = Base.share_mem.nSendBufferCount;
		m_nSendBufferSize = Base.share_mem.nSendBufferByteSize;
		m_nRecvBufferCount = Base.share_mem.nRecvBufferCount;
		m_nRecvBufferSize = Base.share_mem.nRecvBufferByteSize;

		memcpy(&m_CurBase, &Base, sizeof(ConnectionBase));

		if (m_nSendBufferCount > 0 && !m_pSender)
		{
			void* pSender = (void*) new message_queue
			(open_or_create
				, Base.share_mem.szSendIpcName							//name
				, m_nSendBufferCount									//max message number
				, m_nSendBufferSize + sizeof(MSG_QUEUE_STRUCT_INFO)		//max message size
			);
			m_pSender = pSender;
		}

		if (m_nRecvBufferCount > 0 && !m_pRecver)
		{
			void* pRecver = (void*) new message_queue
			(open_or_create
				, Base.share_mem.szRecvIpcName							//name
				, m_nRecvBufferCount									//max message number
				, m_nRecvBufferSize + sizeof(MSG_QUEUE_STRUCT_INFO)		//max message size
			);
			m_pRecver = pRecver;
			StartRecv();
		}

		m_bIsTryConnect = true;
	}
	catch (interprocess_exception ex)
	{
		message_queue::remove(Base.share_mem.szSendIpcName);
		message_queue::remove(Base.share_mem.szRecvIpcName);
		std::cout << ex.what() << std::endl;
		return 1;
	}
	return true;
}

bool __ENF::CONNECTOR::SharedMemory::Disconnect()
{
	if (_SENDER)
	{
		trace_("[SharedMemory] disconnect - sender remove .. \n");
		_SENDER->remove(m_CurBase.share_mem.szSendIpcName);
		trace_("[SharedMemory] disconnect - sender remove done, delete .. \n");
		delete _SENDER;
		trace_("[SharedMemory] disconnect - sender remove done, delete done \n");
		m_pSender = nullptr;
	}
	if (_RECVER)
	{
		trace_("[SharedMemory] disconnect - stop recv .. \n");
		StopRecv();
		trace_("[SharedMemory] disconnect - stop recv done \n");

		trace_("[SharedMemory] disconnect - receiver remove .. \n");
		_RECVER->remove(m_CurBase.share_mem.szRecvIpcName);
		trace_("[SharedMemory] disconnect - receiver remove done, delete .. \n");
		delete _RECVER;
		trace_("[SharedMemory] disconnect - receiver remove done, delete done \n");
		m_pRecver = nullptr;
	}
	trace_("[SharedMemory] disconnect - assist clear .. \n");
	if (_RECV_ASSIST)
		_RECV_ASSIST->clear();
	trace_("[SharedMemory] disconnect - assist clear done \n");
	m_bIsTryConnect = false;

	return true;
}

bool __ENF::CONNECTOR::SharedMemory::Reconnect()
{
	if (strlen(m_CurBase.share_mem.szSendIpcName) <= 0 && strlen(m_CurBase.share_mem.szRecvIpcName) <= 0)
		return false;

	Disconnect();
	return Connect(m_CurBase);
}

bool __ENF::CONNECTOR::SharedMemory::IsConnectionEstablished()
{
	return m_bIsTryConnect;
}

void __ENF::CONNECTOR::SharedMemory::StartRecv()
{
	if (!m_ptRecv)
	{
		m_bIsTryingRecv = true;
		m_ptRecv = (void*)new std::thread(std::bind(&__ENF::CONNECTOR::SharedMemory::RecvFunc, this));
	}
}

void __ENF::CONNECTOR::SharedMemory::StopRecv()
{
	if (m_ptRecv)
	{
		m_bIsTryingRecv = false;
		// 최우선순위 종료 메세지 전달.
		notifyRecvThread();
		if (_RECV_THREAD->joinable())
			_RECV_THREAD->join();
		delete _RECV_THREAD;
		m_ptRecv = nullptr;
	}

}

bool __ENF::CONNECTOR::SharedMemory::SendMessage(void* pTarget, unsigned char* pBuffer, const size_t& nSize, unsigned int nPriority, int nSendOption)
{
	try
	{
		bool bsendret = true;
		if (!pTarget)
			return false;

		if (!pBuffer || nSize <= 0)
			return false;
		trace_("[SharedMemory] send #1\n");
		MSG_QUEUE_STRUCT_INFO stmessage;
		stmessage.nZeroBaseMessageID = (*_MESSAGE_ID)++;
		stmessage.tp = std::chrono::steady_clock::now();

		if (nSize < m_nSendBufferSize)
		{
			stmessage.nZeroBaseMaxMessageIndex = 0;
			stmessage.nZeroBaseCurMessageIndex = 0;

			const size_t newsize = sizeof(MSG_QUEUE_STRUCT_INFO) + nSize;
			unsigned char* pSendBuffer = new unsigned char[newsize];
			memcpy(pSendBuffer, &stmessage, sizeof(stmessage));
			memcpy(pSendBuffer + sizeof(stmessage), pBuffer, nSize);

			switch (nSendOption)
			{
			case 1:
				trace_("[SharedMemory] send #2-1-1\n");
				CONV_QUEUE(pTarget)->send(pSendBuffer, newsize, nPriority);
				trace_("[SharedMemory] send #2-1-2\n");
				break;
			case 2:
			{
				trace_("[SharedMemory] send #2-2-1\n");
				boost::posix_time::ptime abs_time = boost::posix_time::microsec_clock::universal_time() + boost::posix_time::millisec(m_nSendTimeout);
				bsendret &= CONV_QUEUE(pTarget)->timed_send(pSendBuffer, newsize, nPriority, abs_time);
				trace_("[SharedMemory] send #2-2-2\n");
				break;
			}
			case 0:
			default:
				trace_("[SharedMemory] send #2-3-1\n");
				bsendret &= CONV_QUEUE(pTarget)->try_send(pSendBuffer, newsize, nPriority);
				trace_("[SharedMemory] send #2-3-2\n");
				break;
			}

			if (pSendBuffer)
			{
				delete[] pSendBuffer;
				pSendBuffer = nullptr;
			}
		}
		else
		{
			// 쪼개어 보내질 데이터 갯수
			const unsigned int nToSendCount = (unsigned int)((nSize / m_nSendBufferSize) + (nSize % m_nSendBufferSize > 0 ? 1 : 0));
			
			if (nToSendCount > m_nSendBufferCount)
			{
#ifdef _enf_assert
				char msg[128] = { NULL, };
				sprintf_s(msg, "Buffer count is to small !. data size: %llu, send buffer size: %u, send buffer count: %u, to send: %u"
					, nSize, m_nSendBufferSize, m_nSendBufferCount, nToSendCount
				);
				_enf_assert(false, msg);
#endif
				return false;
			}

			stmessage.nZeroBaseMaxMessageIndex = nToSendCount - 1;

			unsigned char* pSendBuffer = new unsigned char[m_nSendBufferSize + sizeof(MSG_QUEUE_STRUCT_INFO)];
			message_queue::size_type sended_size = 0;
			for (unsigned int i = 0; i < nToSendCount; i++)
			{
				stmessage.nZeroBaseCurMessageIndex = i;
				const size_t nRemain = nSize - sended_size;
				const size_t nCopy = nRemain >= m_nSendBufferSize ? m_nSendBufferSize : nRemain;

				memcpy(pSendBuffer, &stmessage, sizeof(stmessage));
				memcpy(pSendBuffer + sizeof(stmessage)
					, pBuffer + (m_nSendBufferSize * i)
					, nCopy);

				switch (nSendOption)
				{
				case 1:
					trace_("[SharedMemory] send #3-1-1\n");
					CONV_QUEUE(pTarget)->send(pSendBuffer, sizeof(stmessage) + nCopy, nPriority);
					trace_("[SharedMemory] send #3-1-2\n");
					break;
				case 2:
				{
					trace_("[SharedMemory] send #3-2-1\n");
					boost::posix_time::ptime abs_time = boost::posix_time::microsec_clock::universal_time() + boost::posix_time::millisec(m_nSendTimeout);
					bsendret &= CONV_QUEUE(pTarget)->timed_send(pSendBuffer, sizeof(stmessage) + nCopy, nPriority, abs_time);
					trace_("[SharedMemory] send #3-2-2\n");
					break;
				}
				case 0:
					trace_("[SharedMemory] send #3-3-1\n");
					bsendret &= CONV_QUEUE(pTarget)->try_send(pSendBuffer, sizeof(stmessage) + nCopy, nPriority);
					trace_("[SharedMemory] send #3-3-2\n");
				default:
					break;
				}

				sended_size += nCopy;
			}

			if (pSendBuffer)
			{
				delete[] pSendBuffer;
				pSendBuffer = nullptr;
			}
		}
		trace_("[SharedMemory] done. ret: %d\n", bsendret);
		return bsendret;

	}
	catch (const std::system_error &e)
	{
		printf("Shared Memory Send exception. [sys except] code [%d] err[%s]\n"
			, e.code().value()
			, e.what()
		);
	}
	catch (boost::interprocess::interprocess_exception e)
	{
		printf("Shared Memory Send exception. [ipc except] code [%d] err[%s]\n"
			, e.get_error_code()
			, e.what()
		);
	}
	catch (...)
	{
		printf("Shared Memory Send exception. [Unknown exception] \n");
	}
	return false;
}

bool __ENF::CONNECTOR::SharedMemory::Remove()
{
	bool bret1 = false, bret2 = false;
	if(strlen(m_CurBase.share_mem.szSendIpcName) > 0)
		bret1 = Remove(m_CurBase.share_mem.szSendIpcName);

	if (strlen(m_CurBase.share_mem.szRecvIpcName) > 0)
		bret2 = Remove(m_CurBase.share_mem.szRecvIpcName);

	return bret1 && bret2;
}

unsigned long __ENF::CONNECTOR::SharedMemory::GetLastError()
{
	return boost::interprocess::winapi::get_last_error();
}

const int& __ENF::CONNECTOR::SharedMemory::GetSendOption()
{
	return m_SendOption;
}

const int& __ENF::CONNECTOR::SharedMemory::GetRecvOption()
{
	return m_RecvOption;
}

const unsigned int& __ENF::CONNECTOR::SharedMemory::GetSendBufferSize()
{
	return m_nSendBufferSize;
}

const unsigned int& __ENF::CONNECTOR::SharedMemory::GetSendBufferCount()
{
	return m_nSendBufferCount;
}

const unsigned int& __ENF::CONNECTOR::SharedMemory::GetRecvBufferSize()
{
	return m_nRecvBufferSize;
}

const unsigned int& __ENF::CONNECTOR::SharedMemory::GetRecvBufferCount()
{
	return m_nRecvBufferCount;
}

void __ENF::CONNECTOR::SharedMemory::RecvFunc()
{
	unsigned int nPriority = 0;
	const size_t nHeadSize = sizeof(MSG_QUEUE_STRUCT_INFO);
	const size_t nNewBuffersize = m_nRecvBufferSize + nHeadSize;
	unsigned char* pNewBuffer = new unsigned char[nNewBuffersize];
	ZeroMemory(pNewBuffer, nNewBuffersize);
	while (m_bIsTryingRecv)
	{
		trace_("[SharedMemory] recv start cycle..\n");
		//m_pRecver;
		message_queue::size_type recvd_size = 0;
		
		//boost::posix_time::ptime abs_time = boost::posix_time::microsec_clock::universal_time() + boost::posix_time::millisec(m_nSendTimeout);
		//_RECVER->timed_receive(pNewBuffer, nNewBuffersize, recvd_size, nPriority, abs_time);
		try
		{
			switch (m_RecvOption)
			{
			case 0:
				trace_("[SharedMemory] recv #1-1\n");
				if (_RECVER)
					_RECVER->try_receive(pNewBuffer, nNewBuffersize, recvd_size, nPriority);
				trace_("[SharedMemory] recv #1-2\n");
				break;
			case 1:
				trace_("[SharedMemory] recv #2-1\n");
				if (_RECVER)
					_RECVER->receive(pNewBuffer, nNewBuffersize, recvd_size, nPriority);
				trace_("[SharedMemory] recv #2-2\n");
				break;
			case 2:
				trace_("[SharedMemory] recv #3-1\n");
				if (_RECVER)
				{
					boost::posix_time::ptime abs_time = boost::posix_time::microsec_clock::universal_time() + boost::posix_time::millisec(m_nRecvTimeout);
					_RECVER->timed_receive(pNewBuffer, nNewBuffersize, recvd_size, nPriority, abs_time);
				}
				trace_("[SharedMemory] recv #3-2\n");
				break;
			}
		}
		catch (boost::interprocess::interprocess_exception e)
		{
			trace_("[SharedMemory] recv err\n");
			printf("Shared Memory Recv exception. code [%d] err[%s]\n"
				, e.get_error_code()
				, e.what()
			);
			boost::interprocess::winapi::set_last_error((unsigned long)e.get_error_code());
			continue;
		}
		// 메세지 수신 기능 및 최우선순위 수신시 while문 종료 메세지.
		if (nPriority == UINT_MAX && recvd_size == 0)
		{
			trace_("[SharedMemory] recv size 0, break while ..\n");
			continue;
		}
		if (recvd_size < nHeadSize)
		{
			trace_("[SharedMemory] recv size infficient\n");
			if(m_RecvOption == 0)
				std::this_thread::sleep_for(std::chrono::milliseconds(m_nRecvTimeout));
			continue;
		}

		trace_("[SharedMemory] recv start parsing..\n");
		const MSG_QUEUE_STRUCT_INFO* pHeader = (MSG_QUEUE_STRUCT_INFO * )pNewBuffer;
		if (pHeader)
		{
			// 하나로 보내질 경우
			if (pHeader->nZeroBaseMaxMessageIndex == 0)
			{
				if (m_pPacketReceiveCallback)
					m_pPacketReceiveCallback(pNewBuffer + nHeadSize, (unsigned int)(recvd_size - nHeadSize));
			}
			else
			{
				if (_RECV_ASSIST)
				{
					MSG_QUEUE_STRUCT_INFO_ARRAY starr;
					memcpy(&starr.stmsg, pHeader, nHeadSize);
					starr.vPacket = std::vector<unsigned char>(pNewBuffer + nHeadSize, pNewBuffer + nHeadSize + recvd_size - nHeadSize);

					// 패킷 데이터 만들어서 보관.
					auto clone = ((std::map<unsigned int, std::vector<MSG_QUEUE_STRUCT_INFO_ARRAY>>*)m_pReceiveAssist);
					std::vector<MSG_QUEUE_STRUCT_INFO_ARRAY>& vMapdata = (*_RECV_ASSIST)[pHeader->nZeroBaseMessageID];
					vMapdata.push_back(std::move(starr));

					// 패킷이 모두 수신되었을 경우.
					if (vMapdata.size() >= ((size_t)pHeader->nZeroBaseMaxMessageIndex + 1))
					{
						size_t nTotMsg = 0;
						for (const auto& s : vMapdata)
							nTotMsg += s.vPacket.size();

						/*
						// 전송할 때 인덱스 정렬해서 전송하는데, 굳이 필요 없을 것 같다.
						// 인덱스 순으로 정렬해주고.
						std::sort(vMapdata.begin(), vMapdata.end()
							, [](MSG_QUEUE_STRUCT_INFO_ARRAY& t1, MSG_QUEUE_STRUCT_INFO_ARRAY& t2)
							{
								return t1.stmsg.nZeroBaseCurMessageIndex < t2.stmsg.nZeroBaseCurMessageIndex;
							}
						);
						*/

						std::vector<unsigned char> vPacket;
						vPacket.reserve(nTotMsg);

						for (auto& s : vMapdata)
							vPacket.insert(vPacket.end(), std::make_move_iterator(s.vPacket.begin()), std::make_move_iterator(s.vPacket.end()));

						if (m_pPacketReceiveCallback)
							m_pPacketReceiveCallback(vPacket.data(), (unsigned int)vPacket.size());

						(*_RECV_ASSIST).erase(pHeader->nZeroBaseMessageID);
					}
				}
			}
		}
		trace_("[SharedMemory] recv end parsing..\n");
		trace_("[SharedMemory] recv cycle done..\n");
	}
	if (pNewBuffer)
	{
		delete[] pNewBuffer;
		pNewBuffer = nullptr;
	}
}

void __ENF::CONNECTOR::SharedMemory::notifyRecvThread()
{
	try
	{
		if (_RECVER)
			_RECVER->send(NULL, 0, UINT_MAX);
	}
	catch (boost::interprocess::interprocess_exception e)
	{
		printf("notifyRecvThread - send exception. code [%d] err[%s]\n"
			, e.get_error_code()
			, e.what()
		);
	}
}

bool __ENF::CONNECTOR::SharedMemory::Remove(char* szIpcName)
{
	return message_queue::remove(szIpcName);
}

bool __ENF::CONNECTOR::SharedMemory::SendPacket(unsigned char* pBuffer, const size_t& nSize)
{
	if (m_pSender)
		return SendMessage(m_pSender, pBuffer, (unsigned int)nSize, 0, m_SendOption);

	return false;
}

bool __ENF::CONNECTOR::SharedMemory::ForceSendPacket(unsigned char* pBuffer, const size_t& nSize)
{
	if (m_pSender)
		return SendMessage(m_pSender, pBuffer, (unsigned int)nSize, 0, 1);

	return false;
}

bool __ENF::CONNECTOR::SharedMemory::SendPacket(const std::vector<unsigned char>& vBuffer)
{
	if (m_pSender)
		return SendMessage(m_pSender, (unsigned char*)vBuffer.data(), (unsigned int)vBuffer.size(), 0, m_SendOption);

	return false;
}

bool __ENF::CONNECTOR::SharedMemory::SendPacket(std::vector<unsigned char>&& vBuffer)
{
	if (m_pSender)
		return SendMessage(m_pSender, (unsigned char*)vBuffer.data(), (unsigned int)vBuffer.size(), 0, m_SendOption);

	return false;
}
