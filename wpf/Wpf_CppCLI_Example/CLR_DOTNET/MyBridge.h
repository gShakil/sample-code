// MyBridge.h
#pragma once

using namespace System;
using namespace System::Collections::Generic; // List<T>를 사용하기 위해 필요

// c++cli에서 구조체를 정의하자.
// c#에서 정의하면 c++/cli에서 c#프로젝트를 참조해야 한다.
public value struct structSample
{
    int intvalue;
    float floatvalue;
    String^ stringValue; // 관리형 문자열이 포함되면 '복합 구조체'가 됨.
};

namespace MyCppBridge { // <-- 이 이름이 C#의 using문에 들어가는 이름입니다.
    public ref class BridgeClass { // <-- 반드시 'public'이어야 C#에서 보입니다.
    public:
		String^             GetHelloFromCpp();
		int                 getIntFromCpp();
		float               getfloatFromCpp();
		wchar_t                getcharvalueFromCpp();
        cli::array<int>^    getVectorFromCpp();
        List<int>^          getListVectorFromCpp();
        cli::array<int>^    fastgetVectorFromCpp();

        void setVectorFromCharp(cli::array<int>^ indata);
        void setStructFromCharp(structSample% inoutdata);

        void updateStringValue(String^% inout);
        void updateIntptrValue(int* inout); // unsafe 옵션 필요
        void updateIntrefValue(int& inout); // 사용할 수 없음.
        void updateIntValue(int% inout);


        

    };
}