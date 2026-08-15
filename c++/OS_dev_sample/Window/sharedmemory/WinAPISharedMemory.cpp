#include "pch.h"
#include "WinAPISharedMemory.h"
//#include "CSharedMemory.h"
#include "ControllableSharedMemory.h"
#include <thread>

#define FIX_SIZE (1024 * 1024* 12)
#define CONV_MEM(S) ((ControllableSharedMemory<unsigned char>*)(S))
#define CONV_SENDER CONV_MEM(m_sender)
#define CONV_RECVER CONV_MEM(m_recver)

#define _RECV_THREAD ((std::thread*)m_ptRecv)
struct SharedMemHead
{
	size_t nSize; // SharedMemData.data의 크기.
};
struct SharedMemData
{
	SharedMemHead head;
	unsigned char* data;
};

namespace __ENF
{
	namespace CONNECTOR
	{

		WinAPISharedMemory::WinAPISharedMemory()
			: m_sender(nullptr)
			, m_recver(nullptr)
			, m_ptRecv(nullptr)
			, m_bIsTryingRecv(false)
			, m_bIsCon(false)
		{
			ZeroMemory(&m_CurBase, sizeof(m_CurBase));
		}

		WinAPISharedMemory::~WinAPISharedMemory()
		{
			Disconnect();
		}

		bool WinAPISharedMemory::Connect(const ConnectionBase& Base)
		{
			AllocMemory(Base);
			memcpy(&m_CurBase, &Base, sizeof(ConnectionBase));

			bool b1 = false, b2 = false;

			b1 = OpenMemory(m_sender, Base.share_mem.szSendIpcName);

			if (Base.share_mem.nRecvBufferByteSize > 0)
			{
				b2 = OpenMemory(m_recver, Base.share_mem.szRecvIpcName);
				StartRecv();
			}

			m_bIsCon = b1 && b2;
			return m_bIsCon;
		}

		bool WinAPISharedMemory::Disconnect()
		{
			StopRecv();
			Remove();
			m_bIsCon = false;
			FreeMemory();
			return true;
		}

		bool WinAPISharedMemory::Reconnect()
		{
			if (strlen(m_CurBase.share_mem.szSendIpcName) <= 0 && strlen(m_CurBase.share_mem.szRecvIpcName) <= 0)
				return false;

			Disconnect();
			return Connect(m_CurBase);
		}

		bool WinAPISharedMemory::IsConnectionEstablished()
		{
			return m_bIsCon;
		}

		bool WinAPISharedMemory::SendPacket(unsigned char* pBuffer, const size_t& nSize)
		{
			return sendPakcet(pBuffer, nSize);
		}

		bool WinAPISharedMemory::SendPacket(std::vector<unsigned char>&& vBuffer)
		{
			return sendPakcet(vBuffer.data(), vBuffer.size());
		}

		bool WinAPISharedMemory::SendPacket(const std::vector<unsigned char>& vBuffer)
		{
			return sendPakcet((unsigned char*)vBuffer.data(), vBuffer.size());
		}

		bool WinAPISharedMemory::Remove()
		{
			bool b1 = true, b2 = true;
			if (CONV_SENDER)
				b1 = CONV_SENDER->ResetSharedMemory();
			if (CONV_RECVER)
				b2 = CONV_RECVER->ResetSharedMemory();

			return b1 && b2;
		}

		void WinAPISharedMemory::StartRecv()
		{
			if (!m_ptRecv)
			{
				m_bIsTryingRecv = true;
				m_ptRecv = (void*)new std::thread(std::bind(&WinAPISharedMemory::RecvFunc, this));
			}
		}

		void WinAPISharedMemory::StopRecv()
		{
			if (m_ptRecv)
			{
				m_bIsTryingRecv = false;
				if(_RECV_THREAD)
				{
					if (_RECV_THREAD->joinable())
						_RECV_THREAD->join();
					delete _RECV_THREAD;
				}
				m_ptRecv = nullptr;
			}
		}

// 		bool WinAPISharedMemory::sendPakcet(unsigned char* headerbuf, unsigned char* buf, const size_t& size)
// 		{
// 			if (!CONV_SENDER)
// 				return false;
// 
// 			bool bRet(false);
// 			size_t unSize = sizeof(SharedMemHead) + size;
// 
// 			if (Lock())
// 			{
// 				if (CONV_SENDER->GetFreeSize() >= unSize)
// 				{
// 					AppendPacketNoLock(headerbuf, sizeof(SharedMemHead));
// 					AppendPacketNoLock(buf, size);
// 					bRet = true;
// 				}
// 				Unlock();
// 			}
// 			return bRet;
// 
// 			if (CONV_SENDER->GetFreeSize() >= unSize)
// 			{
// 				if (CONV_SENDER->AddSharedMemory(headerbuf, sizeof(SharedMemHead)))
// 				{
// 					while (!bRet)
// 					{
// 						if (CONV_SENDER->GetFreeSize() >= size)
// 							bRet = CONV_SENDER->AddSharedMemory(buf, size);
// 
// 						if (!bRet)
// 							Sleep(15);
// 
// 					}
// 				}
// 			}
// 
// 			return bRet;
// 		}

		bool WinAPISharedMemory::sendPakcet(unsigned char* buf, const size_t& size)
		{
			if (!CONV_SENDER || !m_bIsCon)
				return false;

			//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("sendPakcet this: 0x%02x.. size: %llu start", this, size));
			//SharedMemHead head;
			//head.nSize = size;

			bool bRet(false);
			size_t unSize = sizeof(SharedMemHead) + size;

			if (Lock())
			{
				if (CONV_SENDER->GetFreeSize() >= unSize)
				{
					SendPacketNoLock(buf, size, size);
					//AppendPacketNoLock((unsigned char*)&head, sizeof(SharedMemHead));
					//AppendPacketNoLock(buf, size);
					bRet = true;
				}
				Unlock();
			}
			//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("sendPakcet this: 0x%02x.. size: %llu end", this, size));
			return bRet;
		}

		bool WinAPISharedMemory::OpenMemory(void* mem, const char* name)
		{
			bool ret = false;

			int nlen = MultiByteToWideChar(CP_ACP, NULL, name, -1, NULL, NULL);
			wchar_t* conv = new wchar_t[nlen];
			MultiByteToWideChar(CP_ACP, NULL, name, -1, conv, nlen);
			CONV_MEM(mem)->SetSharedMemoryName(conv);
			//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("OpenMemory.. mem: 0x%02x SetSharedMemoryName: name: %s conv: %s", mem, name, CStringA(conv).GetBuffer()));
			if (conv)
			{
				delete[] conv;
				conv = NULL;
			}
			if (CONV_MEM(mem)->OpenSharedMemory())
			{
				CONV_MEM(mem)->ResetSharedMemory();
				ret = CONV_MEM(mem)->GetSharedPoint();
			}
			else
			{
				CONV_MEM(mem)->CreateSharedMemory();
				ret = CONV_MEM(mem)->GetSharedPoint();
			}

			return ret;
		}

		void WinAPISharedMemory::RecvFunc()
		{
			UINT	uHeaderSize = sizeof(SharedMemHead);
			DWORD dwRet(0);
			BOOL	bRet(FALSE);

			SharedMemData data;
			ZeroMemory(&data, sizeof(data));
			size_t nCurBufferSize = 0;
			while (m_bIsTryingRecv)
			{
				if (!CONV_RECVER)
				{
					Sleep(10);
					continue;
				}

//				try
//				{
					if (CONV_RECVER->GetDataSize() > uHeaderSize)
					{
						ZeroMemory(&data.head, sizeof(data.head));
						bRet = CONV_RECVER->GetSharedMemory(&data.head, uHeaderSize);
						//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("get header.. ret: %d, data size in header : %llu this: 0x%02x", bRet, data.head.nSize, this));
						//ASSERT(data.head.nSize < 3000000);
						if (bRet)
						{
							bRet = FALSE;
							//메모리 재할당
							if (nCurBufferSize < data.head.nSize)
							{
								if (data.data)
								{
									//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("buffer delete. buf: 0x%02x this: 0x%02x", data.data, this));
									delete[] data.data;
									data.data = nullptr;
								}
								//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("buffer creating .. buf: 0x%02x size: %llu, this: 0x%02x"
								//	, data.data == nullptr ? 0x00 : data.data
								//	, data.head.nSize
								//	, this
								//));

								if(data.head.nSize > 0)
									data.data = new unsigned char[data.head.nSize];
								nCurBufferSize = data.head.nSize;
								//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("buffer recreate. buf: 0x%02x size: %llu this: 0x%02x"
								//	, data.data == nullptr ? 0x00 : data.data
								//	, data.head.nSize
								//	, this
								//));
							}
							//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("buffer memset. buf: 0x%02x this: 0x%02x", data.data, this));
							if( data.data != nullptr)
								ZeroMemory(data.data, nCurBufferSize);

							while (!bRet && data.head.nSize > 0)
							{
								if (!m_bIsTryingRecv) break;

								if (CONV_RECVER->GetDataSize() >= data.head.nSize)
									bRet = CONV_RECVER->GetSharedMemory(data.data, data.head.nSize);
								else
								{
									//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("data wait .. buf: 0x%02x recved size: %llu, data.head.nSize: %llu this: 0x%02x"
									//	, data.data
									//	, CONV_RECVER->GetDataSize()
									//	, data.head.nSize
									//	, this
									//));
								}

								if (!bRet)Sleep(15);
							}
						}
						else
						{
							//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("GetSharedMemory fail .. this: 0x%02x", this));
						}
					}

					if (bRet && data.head.nSize > 0)
					{
						if (m_pPacketReceiveCallback)
							m_pPacketReceiveCallback(data.data, (unsigned int)data.head.nSize);
							//m_pPacketReceiveCallback((unsigned char*)&data, (unsigned int)(sizeof(SharedMemData) + data.head.nSize - sizeof(data.data)));
						bRet = FALSE;
					}
					else
						Sleep(15);
//				}
// 				catch (...)
// 				{
// 					//TRACE("memory exception\n");
// 					//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("memory exception .."));
// 				}
			}

			if (data.data)
			{
				delete[] data.data;
				data.data = nullptr;
			}

		}

		bool WinAPISharedMemory::SendPacketNoLock(unsigned char* pBuffer, const size_t& nSize, const size_t& totSize)
		{
			if (!CONV_SENDER || !m_bIsCon)
				return false;

			//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("SendPacketNoLock this: 0x%02x.. size: %llu totSize: %llu start", this, nSize, totSize));
			SharedMemHead head;
			head.nSize = totSize;

			bool bRet(false);

			size_t unSize = sizeof(SharedMemHead) + totSize;

			if (CONV_SENDER->GetFreeSize() >= unSize)
			{
				if (CONV_SENDER->AddSharedMemoryNoLock(&head, sizeof(SharedMemHead)))
				{
					while (!bRet)
					{
						if (CONV_SENDER->GetFreeSize() >= nSize)
							bRet = CONV_SENDER->AddSharedMemoryNoLock(pBuffer, nSize);

						if (!bRet)
							Sleep(15);

					}
				}
			}
			//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("SendPacketNoLock this: 0x%02x.. size: %llu totSize: %llu end", this, nSize, totSize));
			return bRet;
		}

		bool WinAPISharedMemory::AppendPacketNoLock(unsigned char* pBuffer, const size_t& nSize)
		{
			if (!CONV_SENDER || !m_bIsCon)
				return false;

			//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("AppendPacketNoLock this: 0x%02x.. size: %llu  start", this, nSize));
			bool bRet(FALSE);
			while (!bRet)
			{
				if (CONV_SENDER->GetFreeSize() >= nSize)
					bRet = CONV_SENDER->AddSharedMemoryNoLock(pBuffer, nSize);

				if (!bRet)
					Sleep(15);

			}
			//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("AppendPacketNoLock this: 0x%02x.. size: %llu  end", this, nSize));
			return bRet;
		}

		bool WinAPISharedMemory::Lock()
		{
			bool ret = false;
			//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("try WinAPISharedMemory::LOCK .. m_sender: 0x%02x", m_sender));
			if (CONV_SENDER && m_bIsCon)
			{
				ret = CONV_SENDER->Lock();
				//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("try WinAPISharedMemory::LOCK end m_sender: 0x%02x, ret: %d", m_sender, ret));
			}

			//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("try LOCK .. ret: %d", ret));
			return ret;
		}

		bool WinAPISharedMemory::Unlock()
		{
			bool ret = false;
			//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("try WinAPISharedMemory::UNLOCK .. m_sender: 0x%02x", m_sender));
			if (CONV_SENDER && m_bIsCon)
			{
				ret = CONV_SENDER->Unlock();
				//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("try WinAPISharedMemory::UNLOCK end m_sender: 0x%02x, ret: %d", m_sender, ret));
			}
			//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("try UNLOCK .. ret: %d", ret));
			return ret;
		}

		void WinAPISharedMemory::AllocMemory(const ConnectionBase& Base)
		{
			m_sender = (void*)new ControllableSharedMemory<unsigned char>(Base.share_mem.nSendBufferByteSize);
			m_recver = (void*)new ControllableSharedMemory<unsigned char>(Base.share_mem.nRecvBufferByteSize);
		}

		void WinAPISharedMemory::FreeMemory()
		{
			if (CONV_MEM(m_sender))
			{
				delete CONV_MEM(m_sender);
				m_sender = nullptr;
			}
			if (CONV_MEM(m_recver))
			{
				delete CONV_MEM(m_recver);
				m_recver = nullptr;
			}
		}

	}
}