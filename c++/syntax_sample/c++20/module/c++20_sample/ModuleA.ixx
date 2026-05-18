// export module ModuleA;
// import <iostream>;
// 
// export void MyFunc()
// {
// 	std::cout << "I`m Module A: MyFunc" << std::endl;
// }


// helloworld.cpp
export module ModuleA; // module declaration

import <iostream>;        // import declaration

export void MyFunc()       // export declaration
{
	std::cout << "Hello world!\n";
}