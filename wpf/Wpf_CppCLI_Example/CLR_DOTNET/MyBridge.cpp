#include "MyBridge.h"
#include <vector>

inline String^ MyCppBridge::BridgeClass::GetHelloFromCpp() { 
	return "Success!"; 

}

void MyCppBridge::BridgeClass::updateStringValue(String^% inout)
{
	inout = "update string by cpp";
}

void MyCppBridge::BridgeClass::updateIntptrValue(int* inout)
{
	*inout = -88;
}

void MyCppBridge::BridgeClass::updateIntrefValue(int& inout)
{
	inout = -99;
}

void MyCppBridge::BridgeClass::updateIntValue(int% inout)
{
	inout = -77;
}

int MyCppBridge::BridgeClass::getIntFromCpp()
{
	return -3;
}

float MyCppBridge::BridgeClass::getfloatFromCpp()
{
	return -7.0f;
}

wchar_t MyCppBridge::BridgeClass::getcharvalueFromCpp()
{
	return L'A';
}

cli::array<int>^ MyCppBridge::BridgeClass::getVectorFromCpp()
{
	std::vector<int> vdata = {1,2,3,4,5,6,5,4,3,2,1};
	cli::array<int>^ result = gcnew cli::array<int>(vdata.size());
	for (size_t i = 0; i < vdata.size(); i++)
		result[i] = vdata[i];
	return result;
}

List<int>^ MyCppBridge::BridgeClass::getListVectorFromCpp()
{
	std::vector<int> vdata = { 1,2,3,4,5,6,5,4,3,2,1 };
	List<int>^ result = gcnew List<int>(vdata.size()); // 미리 용량 확보해두기.
	for (size_t i = 0; i < vdata.size(); i++)
		result->Add(vdata[i]);

	return result;
}

cli::array<int>^ MyCppBridge::BridgeClass::fastgetVectorFromCpp()
{
	std::vector<int> data = { 1,2,3,4,5,6,5,4,3,2,1 };
	cli::array<int>^ outdata = gcnew cli::array<int>(data.size());
	if (data.size() > 0)
	{
		// gc가 메모리를 옮기지 못하도록 고정.
		pin_ptr<int> pdest = &outdata[0];
		// memcpy 복사
		memcpy(pdest, data.data(), data.size() * sizeof(int));
	}

	return outdata;

}

void MyCppBridge::BridgeClass::setVectorFromCharp(cli::array<int>^ indata)
{
	std::vector<int> copydata(indata->Length);
	for (int i = 0; i < indata->Length; i++)
		copydata[i] = indata[i];
}

void MyCppBridge::BridgeClass::setStructFromCharp(structSample% inoutdata)
{
	inoutdata.intvalue = 12323;
	inoutdata.floatvalue = 123.456f;
	inoutdata.stringValue = "struct string value from cpp";
}
