/*
입력되는 데이터를 템플릿으로 받아서 리스트 상,하 이동 및
데이터 추가/수정/삭제 기능을 수행하는 간단한 클래스.
데이터 보유, 수정, 삭제와 이동만 담당하고,
리스트에 출력되는 텍스트는 관여하지 않는다.
*/

#pragma once
#include <vector>

template <typename T>
class SimpleListupContol:
	public CListCtrl
{
public:
	SimpleListupContol();
	virtual ~SimpleListupContol();

	void SetNewData(int nListIndex, const T& data);
	T*	 GetData(int nListIndex);

	void ModifyData(int nListIndex, const T& data);
	void DeleteData(int nListIndex);
	void DeleteAllData();
	bool IsExistData(int nListIndex);
	std::vector<T*> GetDataList(); // 포인터 리턴. 리스트에 등록된 데이터를 가져온다. delete하는 경우 주의.

	void SwapData(int nsrcIndex, int ndestIndex);
	void MoveDataUpper(int nListIndex);
	void MoveDataLower(int nListIndex);


protected:

private:
	bool m_bNeedRelease;
};

//////////////////////////////////////////////////////////////////////////
////////////////////////////////  구현  //////////////////////////////////
//////////////////////////////////////////////////////////////////////////

template <typename T>
SimpleListupContol<T>::SimpleListupContol()
	:m_bNeedRelease(false)
{

}


template <typename T>
SimpleListupContol<T>::~SimpleListupContol()
{
	ASSERT(!m_bNeedRelease); // need to call DeleteAllData()
}

template <typename T>
void SimpleListupContol<T>::SetNewData(int nListIndex, const T& data)
{
	ASSERT(GetItemCount() > nListIndex);
	if (GetItemCount() > nListIndex)
	{
		if (IsExistData(nListIndex))
		{
			DeleteData(nListIndex);
		}

		T* p = new T(data);
		m_bNeedRelease = true;

		SetItemData(nListIndex, (DWORD_PTR)p);
	}
}

template <typename T>
T* SimpleListupContol<T>::GetData(int nListIndex)
{
	ASSERT(GetItemCount() > nListIndex);
	if (GetItemCount() > nListIndex)
	{
		return (T*)GetItemData(nListIndex);
	}

	return NULL;
}

template <typename T>
void SimpleListupContol<T>::ModifyData(int nListIndex, const T& data)
{
	ASSERT(GetItemCount() > nListIndex);
	if (GetItemCount() > nListIndex)
	{
		if (IsExistData(nListIndex))
		{
			T* p = (T*)GetItemData(nListIndex);
			*p = data;
		}
	}
}

template <typename T>
void SimpleListupContol<T>::DeleteData(int nListIndex)
{
	ASSERT(GetItemCount() > nListIndex);
	if (GetItemCount() > nListIndex)
	{
		if (IsExistData(nListIndex))
		{
			T* p = (T*)GetItemData(nListIndex);
			delete p;
			SetItemData(nListIndex, NULL);
		}
	}

	std::vector<T*> vp = GetDataList();
	std::vector<T*>::iterator find = std::find_if(vp.begin(), vp.end(),
		[](T* p)
		{
			return p != NULL;
		}
	);
	if (find == vp.end())
		m_bNeedRelease = false;
}

template <typename T>
void SimpleListupContol<T>::DeleteAllData()
{
	int ncount = GetItemCount();
	for (int i = 0; i < ncount; i++)
	{
		if (IsExistData(i))
			DeleteData(i);
	}
	m_bNeedRelease = false;
}

template <typename T>
bool SimpleListupContol<T>::IsExistData(int nListIndex)
{
	ASSERT(GetItemCount() > nListIndex);
	if (GetItemCount() > nListIndex)
	{
		DWORD_PTR p = GetItemData(nListIndex);
		return p != NULL;
	}

	return false;
}

template <typename T>
std::vector<T*> SimpleListupContol<T>::GetDataList()
{
	std::vector<T*> vList;
	const int count = GetItemCount();
	for (int i = 0; i < count; i++)
		vList.push_back((T*)GetItemData(i));
	return vList;
}

template <typename T>
void SimpleListupContol<T>::SwapData(int nsrcIndex, int ndestIndex)
{
	const int nItemcount = GetItemCount();
	if (
		nsrcIndex < 0 
		|| ndestIndex < 0 
		|| nsrcIndex >= nItemcount  
		|| ndestIndex	>= nItemcount)
		return;

		T* src = (T*)GetItemData(nsrcIndex);
		T* dest = (T*)GetItemData(ndestIndex);

		std::vector<CString> vsrc, vdest;
		const int nsubitemcount = GetHeaderCtrl()->GetItemCount();
		for (int i = 0; i < nsubitemcount; i++)
		{
			CString ssrc, sdest;
			ssrc = GetItemText(nsrcIndex, i);
			sdest = GetItemText(ndestIndex, i);

			vsrc.push_back(ssrc);
			vdest.push_back(sdest);
		}

		SetItemData(ndestIndex, (DWORD_PTR)src);
		SetItemData(nsrcIndex, (DWORD_PTR)dest);

		for (int i = 0; i < nsubitemcount; i++)
		{
			SetItemText(nsrcIndex, i, vdest[i]);
			SetItemText(ndestIndex, i, vsrc[i]);
		}

	UINT nState = GetItemState(nsrcIndex, TVIF_STATE); //아이템의 상태를 가져온다.
	nState &= ~TVIS_SELECTED; //선택 해제
	SetItemState(nsrcIndex, nState, -1);

	nState = GetItemState(ndestIndex, TVIF_STATE); //아이템의 상태를 가져온다.
	nState |= TVIS_SELECTED; //선택
	SetItemState(ndestIndex, nState, -1); //선택
	SetFocus();
	SetSelectionMark(ndestIndex);
}

template <typename T>
void SimpleListupContol<T>::MoveDataUpper(int nListIndex)
{
	SwapData(nListIndex, nListIndex - 1);
}

template <typename T>
void SimpleListupContol<T>::MoveDataLower(int nListIndex)
{
	SwapData(nListIndex, nListIndex + 1);
}