/*
c++ 20에 도입된 module의 샘플 소스. 
샘플이라기엔 미흡하고, 보완이 더 필요하다.
*/


//#include <iostream>
import ModuleB;
import ModuleA;

import speech;

int main()
{
	MyFunc(); // 같은 MyFunc이 ModuleA, ModuleB에 있다면 먼저 import한 모듈의 MyFunc이 호출된다.
	MyFunc();
	
	GetPhraseEN();
	GetPhraseKR();
}
