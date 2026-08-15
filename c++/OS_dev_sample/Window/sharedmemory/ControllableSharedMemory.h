#if _MSC_VER >= 1000
#pragma once
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include <afxmt.h>

template<typename T>
class ControllableSharedMemory
{
public:
	ControllableSharedMemory(size_t dataArrayCount)
		:m_dataArrayCount(dataArrayCount)
	{
		m_hMemMap = INVALID_HANDLE_VALUE;
		ZeroMemory(m_szIpcName, sizeof(CHAR[MAX_PATH]));
		ZeroMemory(m_szMutexName, sizeof(CHAR[MAX_PATH]));
		m_pSh = NULL;
		m_unMaxSize = sizeof(T) * dataArrayCount;
		m_dwTimeout = INFINITE;
		m_pQueue = NULL;
	}

	virtual ~ControllableSharedMemory()
	{
		SetUse(FALSE);
		if (m_pQueue != NULL)
		{
			UnmapViewOfFile(m_pQueue);
		}
		if (m_hMemMap != INVALID_HANDLE_VALUE)CloseHandle(m_hMemMap);

		m_pQueue = NULL;
		m_szDatapos = NULL;
		m_hMemMap = INVALID_HANDLE_VALUE;
		if (m_pSh)
		{
			m_pSh->Unlock();
			delete m_pSh;
			m_pSh = NULL;
		}
		ZeroMemory(m_szIpcName, sizeof(CHAR[MAX_PATH]));
		ZeroMemory(m_szMutexName, sizeof(CHAR[MAX_PATH]));
	}
private:
#pragma pack(push, 1)
	typedef struct _CS_QUEUE_
	{
		BOOL		m_bUse;
		unsigned __int64		m_iRear;
		unsigned __int64		m_iFront;
		UINT		m_iCount;
		unsigned __int64		m_sSize;
	} CS_QUEUE, * pCS_QUEUE;
#pragma pack(pop)

	//2010,01,18 IPC Name Mutex Name 수정
	HANDLE		m_hMemMap;
#ifdef UNICODE
	WCHAR		m_szIpcName[MAX_PATH];
#else
	CHAR		m_szIpcName[MAX_PATH];
#endif
	pCS_QUEUE			m_pQueue;
	unsigned __int64	m_unMaxSize;
	DWORD				m_dwTimeout;
	T*					m_szDatapos;
	size_t				m_dataArrayCount;

public:
	CSemaphore* m_pSh;
#ifdef UNICODE
	WCHAR		m_szMutexName[MAX_PATH];
#else
	CHAR		m_szMutexName[MAX_PATH];
#endif

public:

	BOOL	Init(LPCTSTR _strName, DWORD _dwTimeout = INFINITE);

	//공유 메모리 파일 이름을 세팅 한다.
#ifdef UNICODE
	void	SetSharedMemoryName(wchar_t* _szName = NULL)
	{
		wsprintf(m_szIpcName, _T("%s"), _szName);
	}
#else
	void	SetSharedMemoryName(char* _szName = NULL)
	{
		sprintf(m_szIpcName, _T("%s"), _szName);
	}
#endif

	//공유 메모리의 파일을 생성 한다.
	BOOL	CreateSharedMemory(DWORD _dwTimeout = INFINITE);

	//공유 메모리의 파일을 오픈 한다.
	BOOL	OpenSharedMemory(DWORD _dwTimeout = INFINITE);

	//공유 메모리의 포인터를 얻어 온다.
	BOOL	GetSharedPoint();

	BOOL ResetSharedMemory();
	BOOL Lock();
	BOOL Unlock();
	BOOL AddSharedMemoryNoLock(void* _pData = NULL, unsigned __int64 _sSize = 0);
	BOOL AddSharedMemory(T& tData);

	//공유메모리에서 데이터를 가져온다.
	BOOL GetSharedMemory(T& tData);
	BOOL AddSharedMemory(void* _pData = NULL, unsigned __int64 _sSize = 0);
	BOOL GetSharedMemory(void* _pData = NULL, unsigned __int64 _sSize = 0);
	BOOL SetSharedMemory(int _ipos, T& tData);
	BOOL CopySharedMemory(int _ipos, T& tData);

	//공유메모리이 저장된 데이터의 싸이즈를 가져온다.
	unsigned __int64	GetDataSize();
	unsigned __int64	GetFreeSize();
	//공유메모리의 저장된 데이터의 카운트롤 가져온다.
	unsigned __int64	GetDataCnt();
	BOOL	GetUse();
	void	SetUse(BOOL _bUse);

	private:
	size_t getTotalSharedMemSize();
	char* getDataPos(const CS_QUEUE* queue);
};







template<typename T>
char* ControllableSharedMemory<T>::getDataPos(const CS_QUEUE* queue)
{
	return (char*)queue + sizeof(CS_QUEUE);
}

template<typename T>
size_t ControllableSharedMemory<T>::getTotalSharedMemSize()
{
	size_t t = sizeof(CS_QUEUE) + (sizeof(T) * m_dataArrayCount);
	////ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("total Shared Mem Size: %llu. m_unMaxSize: %llu", t, m_unMaxSize));
	return t;
}

template<typename T>
void ControllableSharedMemory<T>::SetUse(BOOL _bUse)
{
	BOOL bRet(FALSE);
	if (m_pQueue)
	{
		m_pQueue->m_bUse = _bUse;
	}
}

template<typename T>
BOOL ControllableSharedMemory<T>::GetUse()
{
	BOOL bRet(FALSE);
	if (m_pQueue)
	{
		bRet = m_pQueue->m_bUse;
	}
	return bRet;
}

template<typename T>
unsigned __int64 ControllableSharedMemory<T>::GetDataCnt()
{
	unsigned __int64 iRet(0);
	if (m_pQueue)
	{
		iRet = m_pQueue->m_iCount;
	}
	return iRet;
}

template<typename T>
unsigned __int64 ControllableSharedMemory<T>::GetFreeSize()
{
	unsigned __int64 uRet(0);
	if (m_pQueue)
	{
		uRet = m_unMaxSize - m_pQueue->m_sSize;
		////ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("GetFreeSize : %llu", uRet));
	}
	return uRet;
}

template<typename T>
unsigned __int64 ControllableSharedMemory<T>::GetDataSize()
{
	unsigned __int64 uRet(0);
	if (m_pQueue)
	{
		uRet = m_pQueue->m_sSize;
	}
	return uRet;
}

template<typename T>
BOOL ControllableSharedMemory<T>::CopySharedMemory(int _ipos, T& tData)
{
	BOOL bRet(FALSE);
	if (m_pQueue)
	{
		if (m_pSh)
		{
			if (m_pSh->Lock(m_dwTimeout))
			{
				if (m_pQueue && _ipos >= 0)
				{
					if (m_pQueue->m_iCount < m_dataArrayCount)
					{
						if (_ipos >= m_dataArrayCount) _ipos = 0;
						CopyMemory(&tData, &m_pQueue->m_szData[_ipos], sizeof(T));
						bRet = TRUE;
					}
				}
				if (m_pSh)m_pSh->Unlock();
			}
		}
	}
	return bRet;
}

template<typename T>
BOOL ControllableSharedMemory<T>::SetSharedMemory(int _ipos, T& tData)
{
	BOOL bRet(FALSE);
	if (m_pQueue)
	{
		if (m_pSh)
		{
			if (m_pSh->Lock(m_dwTimeout))
			{
				if (m_pQueue && _ipos >= 0)
				{
					if (m_pQueue->m_iCount < m_dataArrayCount)
					{
						if (_ipos >= m_dataArrayCount) _ipos = 0;
						CopyMemory(&m_pQueue->m_szData[_ipos], &tData, sizeof(T));
						bRet = TRUE;
					}
				}
				if (m_pSh)m_pSh->Unlock();
			}
		}
	}
	return bRet;
}

template<typename T>
BOOL ControllableSharedMemory<T>::GetSharedMemory(void* _pData /*= NULL*/, unsigned __int64 _sSize /*= 0*/)
{
	BOOL bRet(FALSE);
	if (!_sSize)return bRet;
	if (m_pQueue)
	{
		if (m_pSh)
		{
			////ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("GetSharedMemory .. _pData: 0x%x, _sSize: %llu", _pData, _sSize));
			if (m_pSh->Lock(m_dwTimeout))
			{
				unsigned __int64 ReadSize = m_unMaxSize - m_pQueue->m_iFront;
				unsigned __int64 FrontPos = (unsigned __int64)m_szDatapos + m_pQueue->m_iFront;
				////ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("GetSharedMemory. ReadSize(%llu) = m_unMaxSize(%llu) - m_pQueue->m_iFront(%llu)", ReadSize, m_unMaxSize, m_pQueue->m_iFront));
				//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("GetSharedMemory. FrontPos(0x%x) = m_szDatapos(0x%x) + m_pQueue->m_iRear(%llu)", FrontPos, m_szDatapos, m_pQueue->m_iFront));
				if (m_pQueue->m_sSize >= (UINT)_sSize)
				{
					if (ReadSize >= _sSize)
					{
						LPVOID pSharedMemory = (LPVOID)FrontPos;
						//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("GetSharedMemory #1 _pData: 0x%02x, pSharedMemory: 0x%02x, _sSize: %llu", _pData, pSharedMemory, _sSize));
						CopyMemory(_pData, pSharedMemory, _sSize);
						ZeroMemory(pSharedMemory, _sSize);
						m_pQueue->m_iFront += _sSize;
						if (m_pQueue->m_iFront == m_unMaxSize)m_pQueue->m_iFront = 0;
					}
					else
					{
						LPVOID pSharedMemory = (LPVOID)FrontPos;
						//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("GetSharedMemory #1-2 _pData: 0x%02x, pSharedMemory: 0x%02x, ReadSize: %llu", _pData, pSharedMemory, ReadSize));
						CopyMemory(_pData, pSharedMemory, ReadSize);
						ZeroMemory(pSharedMemory, ReadSize);

						LPVOID pRawData = (LPVOID)((unsigned __int64)_pData + ReadSize);
						//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("GetSharedMemory #1-2 pRawData: 0x%02x, m_szDatapos: 0x%02x, _sSize(%llu) - ReadSize(%llu): %llu", pRawData, m_szDatapos, _sSize, ReadSize));
						CopyMemory(pRawData, m_szDatapos, (_sSize - ReadSize));
						ZeroMemory((void*)m_szDatapos, (_sSize - ReadSize));
						m_pQueue->m_iFront = (unsigned __int64)(_sSize - ReadSize);
					}
					bRet = TRUE;
				}

				if (bRet)m_pQueue->m_sSize -= _sSize;
				if (m_pSh)m_pSh->Unlock();
			}
		}
	}
	return bRet;
}

template<typename T>
BOOL ControllableSharedMemory<T>::GetSharedMemory(T& tData)
{
	BOOL bRet(FALSE);

	if (m_pQueue)
	{
		if (m_pSh)
		{
			if (m_pSh->Lock(m_dwTimeout))
			{
				if (m_pQueue->m_iCount > 0)
				{
					if (m_pQueue->m_iFront >= m_dataArrayCount) m_pQueue->m_iFront = 0;
					CopyMemory(&tData, &m_pQueue->m_szData[m_pQueue->m_iFront], sizeof(T));
					ZeroMemory(&m_pQueue->m_szData[m_pQueue->m_iFront], sizeof(T));
					m_pQueue->m_iFront++;
					m_pQueue->m_iCount--;
					bRet = TRUE;
				}
				if (bRet) m_pQueue->m_sSize -= (UINT)sizeof(T);
				if (m_pSh) m_pSh->Unlock();
			}
		}
	}

	return bRet;
}

template<typename T>
BOOL ControllableSharedMemory<T>::AddSharedMemory(void* _pData /*= NULL*/, unsigned __int64 _sSize /*= 0*/)
{
	BOOL bRet(FALSE);
	if (_pData == NULL || _sSize == 0)return bRet;

	if (m_pQueue)
	{
		if (m_pSh)
		{
			//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("AddSharedMemory .. _pData: 0x%x, _sSize: %llu", _pData, _sSize));
			if (m_pSh->Lock(m_dwTimeout))
			{
				CString strmsg(_T(""));
				if (GetFreeSize() == m_unMaxSize)
				{
					m_pQueue->m_iRear = m_pQueue->m_iFront = 0;
				}

				if (GetFreeSize() >= _sSize)
				{
					unsigned __int64 WriteSize = m_unMaxSize - m_pQueue->m_iRear;
					unsigned __int64 RearPos = (unsigned __int64)m_szDatapos + m_pQueue->m_iRear;
					//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("AddSharedMemory #0. WriteSize(%llu) = m_unMaxSize(%llu) - m_pQueue->m_iRear(%llu)", WriteSize, m_unMaxSize, m_pQueue->m_iRear));
					//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("AddSharedMemory #0. RearPos(0x%x) = m_szDatapos(0x%x) + m_pQueue->m_iRear(%llu)", RearPos, m_szDatapos, m_pQueue->m_iRear));
					if (WriteSize >= _sSize)
					{
						//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("AddSharedMemory #1. RearPos: 0x%x, _pData: 0x%x, _sSize: %llu", RearPos, _pData, _sSize));
						CopyMemory((LPVOID)RearPos, (LPVOID)_pData, _sSize);
						m_pQueue->m_iRear += _sSize;
						if (m_pQueue->m_iRear == m_unMaxSize) m_pQueue->m_iRear = 0;
					}
					else
					{
						LPVOID pRawData = (LPVOID)((unsigned __int64)_pData + WriteSize);
						//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("AddSharedMemory #2-1. RearPos: 0x%x, _pData: 0x%x, WriteSize: %llu", RearPos, _pData, WriteSize));
						CopyMemory((LPVOID)RearPos, (void*)_pData, WriteSize);
						//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("AddSharedMemory #2-2. m_szDatapos: 0x%x, pRawData: 0x%x, _sSize - WriteSize: %llu", m_szDatapos, pRawData, _sSize - WriteSize));
						CopyMemory((LPVOID)m_szDatapos, pRawData, _sSize - WriteSize);
						m_pQueue->m_iRear = (unsigned __int64)(_sSize - WriteSize);
					}
					bRet = TRUE;
				}
				if (bRet)m_pQueue->m_sSize += _sSize;
				if (m_pSh)m_pSh->Unlock();
			}
		}
	}

	return bRet;
}

template<typename T>
BOOL ControllableSharedMemory<T>::AddSharedMemory(T& tData)
{
	BOOL bRet(FALSE);
	if (m_pQueue)
	{
		if (m_pSh)
		{
			if (m_pSh->Lock(m_dwTimeout))
			{
				if (m_pQueue->m_iCount == 0)
				{
					m_pQueue->m_iRear = m_pQueue->m_iFront = 0;
				}

				if (m_pQueue->m_iCount < m_dataArrayCount)
				{
					if (m_pQueue->m_iRear >= m_dataArrayCount)m_pQueue->m_iRear = 0;
					CopyMemory(&m_pQueue->m_szData[m_pQueue->m_iRear], &tData, sizeof(T));
					m_pQueue->m_iRear++;
					m_pQueue->m_iCount++;
					bRet = TRUE;
				}
				if (bRet)m_pQueue->m_sSize += (UINT)sizeof(T);
				if (m_pSh)m_pSh->Unlock();
			}
		}
	}
	return bRet;
}

template<typename T>
BOOL ControllableSharedMemory<T>::AddSharedMemoryNoLock(void* _pData /*= NULL*/, unsigned __int64 _sSize /*= 0*/)
{
	BOOL bRet(FALSE);
	if (_pData == NULL || _sSize == 0)return bRet;

	if (m_pQueue)
	{
		//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("AddSharedMemoryNoLock .. _pData: 0x%x, _sSize: %llu", _pData, _sSize));
		CString strmsg(_T(""));
		if (GetFreeSize() == m_unMaxSize)
		{
			m_pQueue->m_iRear = m_pQueue->m_iFront = 0;
		}

		if (GetFreeSize() >= _sSize)
		{
			unsigned __int64 WriteSize = m_unMaxSize - m_pQueue->m_iRear;
			unsigned __int64 RearPos = (unsigned __int64)m_szDatapos + m_pQueue->m_iRear;
			//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("Copy Memory #0. WriteSize(%llu) = m_unMaxSize(%llu) - m_pQueue->m_iRear(%llu)", WriteSize, m_unMaxSize, m_pQueue->m_iRear));
			//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("Copy Memory #0. RearPos(0x%x) = m_szDatapos(0x%x) + m_pQueue->m_iRear(%llu)", RearPos, m_szDatapos, m_pQueue->m_iRear));
			if (WriteSize >= _sSize)
			{
				//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("Copy Memory #1. RearPos: 0x%x, _pData: 0x%x, _sSize: %llu", RearPos, _pData, _sSize));
				CopyMemory((LPVOID)RearPos, (LPVOID)_pData, _sSize);
				m_pQueue->m_iRear += _sSize;
				if (m_pQueue->m_iRear == m_unMaxSize) m_pQueue->m_iRear = 0;
			}
			else
			{
				LPVOID pRawData = (LPVOID)((unsigned __int64)_pData + WriteSize);
				//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("Copy Memory #2-1. RearPos: 0x%x, _pData: 0x%x, WriteSize: %llu", RearPos, _pData, WriteSize));
				CopyMemory((LPVOID)RearPos, (void*)_pData, WriteSize);
				//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("Copy Memory #2-2. m_szDatapos: 0x%x, pRawData: 0x%x, _sSize - WriteSize: %llu", m_szDatapos, pRawData, _sSize - WriteSize));
				CopyMemory((LPVOID)m_szDatapos, pRawData, _sSize - WriteSize);
				m_pQueue->m_iRear = (unsigned __int64)(_sSize - WriteSize);
			}
			bRet = TRUE;
		}
		if (bRet)m_pQueue->m_sSize += _sSize;
		//if (m_pSh)m_pSh->Unlock();
	}

	return bRet;
}

template<typename T>
BOOL ControllableSharedMemory<T>::Unlock()
{
	if (m_pSh)
	{
		//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("try Controllable::UNLOCK start.. this: 0x%02x t.id: 0x%02x m_pSh: 0x%02x m_szMutexName: %s, m_szIpcName: %s",this, GetCurrentThreadId(), m_pSh, CStringA(m_szMutexName).GetBuffer(), CStringA(m_szIpcName).GetBuffer()));
		BOOL ret = m_pSh->Unlock();
		//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("try Controllable::UNLOCK end.. this: 0x%02x t.id: 0x%02x m_pSh: 0x%02x ret: %d m_szMutexName: %s, m_szIpcName: %s",this, GetCurrentThreadId(), m_pSh, ret, CStringA(m_szMutexName).GetBuffer(), CStringA(m_szIpcName).GetBuffer()));
	}
	return FALSE;
}

template<typename T>
BOOL ControllableSharedMemory<T>::Lock()
{
	if (m_pSh)
	{
		BOOL ret = FALSE;
		//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("try Controllable::LOCK start.. this: 0x%02x. t.id: 0x%02x m_pSh: 0x%02x, m_szMutexName: %s, m_szIpcName: %s", this, GetCurrentThreadId(),  m_pSh, CStringA(m_szMutexName).GetBuffer(), CStringA(m_szIpcName).GetBuffer()));
		ret = m_pSh->Lock(m_dwTimeout);
		//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("try Controllable::LOCK end.. this: 0x%02x t.id: 0x%02x m_pSh: 0x%02x, ret: %d m_szMutexName: %s, m_szIpcName: %s", this, GetCurrentThreadId(), m_pSh, ret, CStringA(m_szMutexName).GetBuffer(), CStringA(m_szIpcName).GetBuffer()));
		return ret;
	}
	return FALSE;
}

template<typename T>
BOOL ControllableSharedMemory<T>::ResetSharedMemory()
{
	BOOL bRet(FALSE);
	if (m_pSh)
	{
		if (m_pSh->Lock(m_dwTimeout))
		{
			if (m_pQueue)
			{
				UnmapViewOfFile(m_pQueue);
				if (m_hMemMap != INVALID_HANDLE_VALUE)CloseHandle(m_hMemMap);
				m_pQueue = NULL;
				m_szDatapos = NULL;
				m_hMemMap = INVALID_HANDLE_VALUE;
				if (CreateSharedMemory())
				{
					bRet = GetSharedPoint();
				}

			}
			if (m_pSh)m_pSh->Unlock();
		}
	}
	return bRet;
}

template<typename T>
BOOL ControllableSharedMemory<T>::GetSharedPoint()
{
	BOOL bRet(FALSE);

	if (m_hMemMap != NULL && m_hMemMap != INVALID_HANDLE_VALUE)
	{
		m_pQueue = (pCS_QUEUE)MapViewOfFile(m_hMemMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		m_szDatapos = (T*)getDataPos(m_pQueue);
		//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("Get Shared Point .. m_szDatapos: 0x%02x", m_szDatapos));

		if (m_pQueue != NULL)bRet = TRUE;
	}
	return bRet;
}

template<typename T>
BOOL ControllableSharedMemory<T>::OpenSharedMemory(DWORD _dwTimeout /*= INFINITE*/)
{
	BOOL bRet(FALSE);
	if (m_hMemMap == INVALID_HANDLE_VALUE || m_hMemMap == NULL)
	{
		m_hMemMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, (LPCTSTR)m_szIpcName);
		DWORD err = GetLastError();
		if (m_hMemMap != NULL && m_hMemMap != INVALID_HANDLE_VALUE)
		{
			bRet = TRUE;
			if (m_pSh == NULL)
			{
#ifdef UNICODE
				wsprintf(m_szMutexName, _T("%s_MUTEX"), m_szIpcName);
#else
				sprintf(m_szMutexName, _T("%s_MUTEX"), m_szIpcName);
#endif
				//m_pSh = new CMyMutex((LPCTSTR)m_szMutexName);
				m_pSh = new CSemaphore(1, 1, (LPCTSTR)m_szMutexName);

				CString s;
				s.Format(_T("OpenSharedMemory: create semaphore m_szMutexName: %s"), (LPCTSTR)m_szMutexName);
				OutputDebugString(s.GetBuffer());
			}
		}
	}

	m_dwTimeout = _dwTimeout;

	return bRet;
}

template<typename T>
BOOL ControllableSharedMemory<T>::CreateSharedMemory(DWORD _dwTimeout /*= INFINITE*/)
{
	BOOL bRet(FALSE);
	//ENF_IF_COND(ENABLE_DEBUG_LOG, print_debug("CreateSharedMemory ..."));
	m_hMemMap = CreateFileMapping(INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		(DWORD)getTotalSharedMemSize(),
		//sizeof(S_QUEUE),
		(LPCTSTR)m_szIpcName);

	if (m_hMemMap != NULL && m_hMemMap != INVALID_HANDLE_VALUE)
	{
		bRet = TRUE;

		if (m_pSh == NULL)
		{
#ifdef UNICODE
			wsprintf(m_szMutexName, _T("%s_MUTEX"), m_szIpcName);
#else
			sprintf(m_szMutexName, _T("%s_MUTEX"), m_szIpcName);
#endif
			m_pSh = new CSemaphore(1, 1, (LPCTSTR)m_szMutexName);
			CString s;
			s.Format(_T("CreateSharedMemory: create semaphore m_szMutexName: %s"), (LPCTSTR)m_szMutexName);
			OutputDebugString(s.GetBuffer());
		}
	}
	else m_hMemMap = INVALID_HANDLE_VALUE;

	m_dwTimeout = _dwTimeout;

	return bRet;
}

template<typename T>
BOOL ControllableSharedMemory<T>::Init(LPCTSTR _strName, DWORD _dwTimeout /*= INFINITE*/)
{
	BOOL bRet(FALSE);
#ifdef UNICODE
	SetSharedMemoryName((wchar_t*)_strName);
#else
	SetSharedMemoryName((char*)_strName);
#endif
	if (OpenSharedMemory(_dwTimeout))
	{
		bRet = GetSharedPoint();
	}
	else
	{
		if (CreateSharedMemory(_dwTimeout))
		{
			bRet = GetSharedPoint();
		}
	}
	return bRet;
}
