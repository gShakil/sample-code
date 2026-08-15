#if _MSC_VER >= 1000
#pragma once
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include <afxmt.h>

template<typename T,UINT iCount>
class CSharedMemory
{
public:
	CSharedMemory()
	{
		m_hMemMap = INVALID_HANDLE_VALUE;
		m_pQueue = NULL;
		ZeroMemory(m_szIpcName,sizeof(CHAR[MAX_PATH]));
		ZeroMemory(m_szMutexName,sizeof(CHAR[MAX_PATH]));
		m_pSh = NULL;
		m_unMaxSize = (unsigned __int64) sizeof(T[iCount]);
		m_dwTimeout = INFINITE;
	}

	virtual ~CSharedMemory()
	{
		SetUse(FALSE);
		if ( m_pQueue != NULL )
		{
			UnmapViewOfFile(m_pQueue);
		}
		if ( m_hMemMap != INVALID_HANDLE_VALUE )CloseHandle(m_hMemMap);

		m_pQueue = NULL;
		m_hMemMap = INVALID_HANDLE_VALUE;
		if(m_pSh)
		{			
			m_pSh->Unlock();
			delete m_pSh;
			m_pSh = NULL;
		}
		ZeroMemory(m_szIpcName,sizeof(CHAR[MAX_PATH]));
		ZeroMemory(m_szMutexName,sizeof(CHAR[MAX_PATH]));
	}
private:

	typedef struct _S_QUEUE_
	{
		BOOL		m_bUse;
		unsigned __int64		m_iRear;
		unsigned __int64		m_iFront;
		UINT		m_iCount;
		unsigned __int64		m_sSize;
		T			m_szData[iCount];
	} S_QUEUE, *pS_QUEUE;
	
	//2010,01,18 IPC Name Mutex Name 수정
	HANDLE		m_hMemMap;
#ifdef UNICODE
	WCHAR		m_szIpcName[MAX_PATH];
#else
	CHAR		m_szIpcName[MAX_PATH];
#endif
	pS_QUEUE			m_pQueue;
	unsigned __int64	m_unMaxSize;
	DWORD				m_dwTimeout;
	
public:
	CSemaphore*			m_pSh;
#ifdef UNICODE
	WCHAR		m_szMutexName[MAX_PATH];
#else
	CHAR		m_szMutexName[MAX_PATH];
#endif

public:

	BOOL	Init(LPCTSTR _strName, DWORD _dwTimeout = INFINITE)
	{
		BOOL bRet(FALSE);
#ifdef UNICODE
		SetSharedMemoryName((wchar_t*)_strName);
#else
		SetSharedMemoryName((char*)_strName);
#endif
		if(OpenSharedMemory(_dwTimeout))
		{
			bRet = GetSharedPoint();
		}
		else
		{
			if(CreateSharedMemory(_dwTimeout))
			{
				bRet = GetSharedPoint();
			}
		}
		return bRet;
	}

	//공유 메모리 파일 이름을 세팅 한다.
#ifdef UNICODE
	void	SetSharedMemoryName( wchar_t * _szName = NULL)
	{
		wsprintf(m_szIpcName,_T("%s"),_szName);
	}
#else
	void	SetSharedMemoryName( char* _szName = NULL)
	{
		sprintf(m_szIpcName,_T("%s"),_szName);
	}
#endif
	

	//공유 메모리의 파일을 생성 한다.
	BOOL	CreateSharedMemory(DWORD _dwTimeout = INFINITE)
	{
		BOOL bRet(FALSE);
		m_hMemMap	= CreateFileMapping(INVALID_HANDLE_VALUE,
										NULL,
										PAGE_READWRITE,
										0,
										sizeof(S_QUEUE),
										(LPCTSTR)m_szIpcName);
		
		if(m_hMemMap != NULL && m_hMemMap != INVALID_HANDLE_VALUE)
		{
			bRet = TRUE;

			if(m_pSh == NULL)
			{
#ifdef UNICODE
				wsprintf(m_szMutexName,_T("%s_MUTEX"),m_szIpcName);
#else
				sprintf(m_szMutexName,_T("%s_MUTEX"),m_szIpcName);
#endif
				m_pSh = new CSemaphore(1,1,(LPCTSTR)m_szMutexName);
			}
		}
		else m_hMemMap = INVALID_HANDLE_VALUE;

		m_dwTimeout = _dwTimeout;

		return bRet;
	}

    //공유 메모리의 파일을 오픈 한다.
	BOOL	OpenSharedMemory(DWORD _dwTimeout = INFINITE)
	{
		BOOL bRet(FALSE);
		if ( m_hMemMap == INVALID_HANDLE_VALUE || m_hMemMap == NULL )
		{
			m_hMemMap	= OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, (LPCTSTR)m_szIpcName);
			DWORD err = GetLastError();
			if(m_hMemMap != NULL && m_hMemMap != INVALID_HANDLE_VALUE)
			{
				bRet = TRUE;
				if(m_pSh == NULL)
				{
#ifdef UNICODE
					wsprintf(m_szMutexName,_T("%s_MUTEX"),m_szIpcName);
#else
					sprintf(m_szMutexName,_T("%s_MUTEX"),m_szIpcName);
#endif
					//m_pSh = new CMyMutex((LPCTSTR)m_szMutexName);
					m_pSh = new CSemaphore(1,1,(LPCTSTR)m_szMutexName);
				}
			}
		}

		m_dwTimeout = _dwTimeout;

		return bRet;
	}

	//공유 메모리의 포인터를 얻어 온다.
	BOOL	GetSharedPoint()
	{
		BOOL bRet(FALSE);

		if(m_hMemMap != NULL && m_hMemMap != INVALID_HANDLE_VALUE)
		{
			m_pQueue  = (pS_QUEUE)MapViewOfFile(m_hMemMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			if(m_pQueue != NULL)bRet = TRUE;
		}
		return bRet;
	}

	BOOL ResetSharedMemory()
	{
		BOOL bRet(FALSE);
		if(m_pSh)
		{
			if(m_pSh->Lock(m_dwTimeout))
			{
				if(m_pQueue)
				{
					UnmapViewOfFile(m_pQueue);
					if ( m_hMemMap != INVALID_HANDLE_VALUE )CloseHandle(m_hMemMap);
					m_pQueue = NULL;
					m_hMemMap = INVALID_HANDLE_VALUE;
					if(CreateSharedMemory())
					{
						bRet = GetSharedPoint();
					}
					
				}
				if(m_pSh)m_pSh->Unlock();
			}
		}
		return bRet;
	}
	BOOL Lock()
	{
		if (m_pSh)
			return m_pSh->Lock(m_dwTimeout);
		return FALSE;
	}
	BOOL Unlock()
	{
		if (m_pSh)
			return m_pSh->Unlock();
		return FALSE;
	}
	BOOL AddSharedMemoryNoLock(void* _pData = NULL, unsigned __int64 _sSize = 0)
	{
		BOOL bRet(FALSE);
		if (_pData == NULL || _sSize == 0)return bRet;

		if (m_pQueue)
		{
			CString strmsg(_T(""));
			if (GetFreeSize() == m_unMaxSize)
			{
				m_pQueue->m_iRear = m_pQueue->m_iFront = 0;
			}

			if (GetFreeSize() >= _sSize)
			{
				unsigned __int64 WriteSize = m_unMaxSize - m_pQueue->m_iRear;
				unsigned __int64 RearPos = (unsigned __int64)m_pQueue->m_szData + m_pQueue->m_iRear;

				if (WriteSize >= _sSize)
				{
					CopyMemory((LPVOID)RearPos, (LPVOID)_pData, _sSize);
					m_pQueue->m_iRear += _sSize;
					if (m_pQueue->m_iRear == m_unMaxSize) m_pQueue->m_iRear = 0;
				}
				else
				{
					LPVOID pRawData = (LPVOID)((unsigned __int64)_pData + WriteSize);
					CopyMemory((LPVOID)RearPos, (void*)_pData, WriteSize);
					CopyMemory((LPVOID)m_pQueue->m_szData, pRawData, _sSize - WriteSize);
					m_pQueue->m_iRear = (unsigned __int64)(_sSize - WriteSize);
				}
				bRet = TRUE;
			}
			if (bRet)m_pQueue->m_sSize += _sSize;
			if (m_pSh)m_pSh->Unlock();
		}

		return bRet;
	}
	BOOL	AddSharedMemory(T &tData)
	{
		BOOL bRet(FALSE);		
		if(m_pQueue)
		{
			if(m_pSh)
			{
				if(m_pSh->Lock(m_dwTimeout))
				{
					if(m_pQueue->m_iCount == 0 )
					{
						m_pQueue->m_iRear = m_pQueue->m_iFront = 0;
					}

					if ( m_pQueue->m_iCount < iCount )
					{
						if ( m_pQueue->m_iRear >= iCount )m_pQueue->m_iRear = 0;
						CopyMemory(&m_pQueue->m_szData[m_pQueue->m_iRear],&tData,sizeof(T));
						m_pQueue->m_iRear++;
						m_pQueue->m_iCount++;
						bRet = TRUE;
					}
					if(bRet)m_pQueue->m_sSize += (UINT)sizeof(T);
					if(m_pSh)m_pSh->Unlock();
				}
			}
		}
		return bRet;
	}
	
	//공유메모리에서 데이터를 가져온다.
	BOOL	GetSharedMemory(T &tData)
	{
		BOOL bRet(FALSE);

		if(m_pQueue)
		{
			if(m_pSh)
			{
				if(m_pSh->Lock(m_dwTimeout))
				{
					if ( m_pQueue->m_iCount > 0 )
					{
						if ( m_pQueue->m_iFront >= iCount ) m_pQueue->m_iFront = 0;
						CopyMemory(&tData,&m_pQueue->m_szData[m_pQueue->m_iFront],sizeof(T));
						ZeroMemory(&m_pQueue->m_szData[m_pQueue->m_iFront],sizeof(T));
						m_pQueue->m_iFront++;
						m_pQueue->m_iCount--;
						bRet = TRUE;
					}
					if(bRet) m_pQueue->m_sSize -= (UINT)sizeof(T);
					if(m_pSh) m_pSh->Unlock();
				}
			}
		}

		return bRet;
	}

	BOOL	AddSharedMemory(void* _pData =  NULL , unsigned __int64 _sSize = 0)
	{
		BOOL bRet(FALSE);
		if(_pData == NULL || _sSize == 0)return bRet;

		if(m_pQueue)
		{
			if(m_pSh)
			{
				if(m_pSh->Lock(m_dwTimeout))
				{
					CString strmsg(_T(""));
					if(GetFreeSize() == m_unMaxSize)
					{
						m_pQueue->m_iRear = m_pQueue->m_iFront = 0;
					}

					if( GetFreeSize() >= _sSize)
					{
						unsigned __int64 WriteSize = m_unMaxSize - m_pQueue->m_iRear;
						unsigned __int64 RearPos = (unsigned __int64)m_pQueue->m_szData + m_pQueue->m_iRear;

						if( WriteSize >= _sSize)
						{
							CopyMemory((LPVOID)RearPos,(LPVOID)_pData,_sSize);
							m_pQueue->m_iRear += _sSize;
							if(m_pQueue->m_iRear == m_unMaxSize) m_pQueue->m_iRear=0;
						}
						else
						{
							LPVOID pRawData = (LPVOID)((unsigned __int64)_pData+WriteSize);
							CopyMemory((LPVOID)RearPos,(void*)_pData,WriteSize);
							CopyMemory((LPVOID)m_pQueue->m_szData, pRawData, _sSize-WriteSize);
							m_pQueue->m_iRear = (unsigned __int64)(_sSize - WriteSize);
						}
						bRet = TRUE;
					}
					if(bRet)m_pQueue->m_sSize += _sSize;
					if(m_pSh)m_pSh->Unlock();
				}
			}
		}
		
		return bRet;
	}

	BOOL GetSharedMemory(void* _pData =  NULL , unsigned __int64 _sSize = 0)
	{
		BOOL bRet(FALSE);
		if(!_sSize)return bRet;
		if(m_pQueue)
		{
			if(m_pSh)
			{
				if(m_pSh->Lock(m_dwTimeout))
				{

					unsigned __int64 ReadSize = m_unMaxSize - m_pQueue->m_iFront;
					unsigned __int64 FrontPos = (unsigned __int64)m_pQueue->m_szData + m_pQueue->m_iFront;

					if(m_pQueue->m_sSize >= (UINT)_sSize)
					{
						if(ReadSize >= _sSize)
						{
							LPVOID pSharedMemory = (LPVOID)FrontPos;
							CopyMemory(_pData,pSharedMemory,_sSize);
							ZeroMemory(pSharedMemory,_sSize);
							m_pQueue->m_iFront += _sSize;
							if(m_pQueue->m_iFront == m_unMaxSize)m_pQueue->m_iFront=0;
						}
						else
						{
							LPVOID pSharedMemory = (LPVOID)FrontPos;
							CopyMemory(_pData,pSharedMemory,ReadSize);
							ZeroMemory(pSharedMemory,ReadSize);

							LPVOID pRawData = (LPVOID)((unsigned __int64)_pData+ReadSize);
							CopyMemory(pRawData,m_pQueue->m_szData,(_sSize - ReadSize));
							ZeroMemory((void*)m_pQueue->m_szData,(_sSize - ReadSize));
							m_pQueue->m_iFront = (unsigned __int64)(_sSize - ReadSize);
						}
						bRet = TRUE;
					}

					if(bRet)m_pQueue->m_sSize -= _sSize;
					if(m_pSh)m_pSh->Unlock();
				}
			}
		}
		return bRet;
	}

	BOOL	SetSharedMemory(int _ipos, T &tData)
	{
		BOOL bRet(FALSE);
		if(m_pQueue)
		{
			if(m_pSh)
			{
				if(m_pSh->Lock(m_dwTimeout))
				{
					if(m_pQueue && _ipos >= 0)
					{
						if ( m_pQueue->m_iCount < iCount )
						{
							if ( _ipos >= iCount ) _ipos = 0;
							CopyMemory(&m_pQueue->m_szData[_ipos],&tData,sizeof(T));
							bRet = TRUE;
						}
					}
					if(m_pSh)m_pSh->Unlock();
				}
			}
		}
		return bRet;
	}

	BOOL	CopySharedMemory(int _ipos, T &tData)
	{
		BOOL bRet(FALSE);
		if(m_pQueue)
		{
			if(m_pSh)
			{
				if(m_pSh->Lock(m_dwTimeout))
				{
					if(m_pQueue && _ipos >= 0)
					{
						if ( m_pQueue->m_iCount < iCount )
						{
							if ( _ipos >= iCount ) _ipos = 0;
							CopyMemory(&tData,&m_pQueue->m_szData[_ipos],sizeof(T));
							bRet = TRUE;
						}
					}
					if(m_pSh)m_pSh->Unlock();
				}
			}
		}
		return bRet;
	}

	//공유메모리이 저장된 데이터의 싸이즈를 가져온다.
	unsigned __int64	GetDataSize()
	{
		unsigned __int64 uRet(0);
		if(m_pQueue)
		{
			uRet = m_pQueue->m_sSize;
		}
		return uRet;
	}

	unsigned __int64	GetFreeSize()
	{
		unsigned __int64 uRet(0);
		if(m_pQueue)
		{
			uRet = m_unMaxSize - m_pQueue->m_sSize;
		}
		return uRet;
	}

	//공유메모리의 저장된 데이터의 카운트롤 가져온다.
	unsigned __int64	GetDataCnt()
	{
		unsigned __int64 iRet(0);
		if(m_pQueue)
		{
			iRet = m_pQueue->m_iCount;
		}
		return iRet;
	}

	BOOL	GetUse()
	{
		BOOL bRet(FALSE);
		if(m_pQueue)
		{
			bRet = m_pQueue->m_bUse;
		}
		return bRet;
	}

	void	SetUse(BOOL _bUse)
	{
		BOOL bRet(FALSE);
		if(m_pQueue)
		{
			m_pQueue->m_bUse= _bUse;
		}
	}
};