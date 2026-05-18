// ConsoleApplication1.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <cstdio>
#include <array>

// Try to include Python headers if available. MSVC supports __has_include.
#if defined(__has_include)
#  if __has_include(<Python.h>)
#    include <Python.h>
#    define HAVE_PYTHON 1
#  else
#    define HAVE_PYTHON 0
#  endif
#else
#  define HAVE_PYTHON 0
#endif

int main()
{
#if HAVE_PYTHON
    Py_SetPythonHome(L"C:\Python-3.14.3\Lib");
    // Initialize the Python interpreter
    Py_Initialize();

    // Ensure current directory is in sys.path so Python_sample.py can be imported
    PyRun_SimpleString("import sys; sys.path.insert(0, '.')");

    // Import Python_sample module
    PyObject* pName = PyUnicode_DecodeFSDefault("Python_sample");
    PyObject* pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (pModule != nullptr) {
        // Get the add function
        PyObject* pFunc = PyObject_GetAttrString(pModule, "add");
        if (pFunc && PyCallable_Check(pFunc)) {
            // Build arguments (3, 5)
            PyObject* pArgs = PyTuple_Pack(2, PyLong_FromLong(3), PyLong_FromLong(5));
            PyObject* pValue = PyObject_CallObject(pFunc, pArgs);
            Py_DECREF(pArgs);
            if (pValue != nullptr) {
                long result = PyLong_AsLong(pValue);
                std::cout << "Python add(3,5) returned: " << result << std::endl;
                Py_DECREF(pValue);
            }
            else {
                PyErr_Print();
                std::cerr << "Call to Python function failed\n";
            }
            Py_DECREF(pFunc);
        }
        else {
            if (PyErr_Occurred()) PyErr_Print();
            std::cerr << "Cannot find function 'add'\n";
        }
        Py_DECREF(pModule);
    }
    else {
        PyErr_Print();
        std::cerr << "Failed to load 'Python_sample' module\n";
    }

    // Finalize the Python interpreter
    Py_Finalize();

    return 0;
#else
    // If Python C headers/libs are not available, run the Python script as a subprocess
    // and print its output. Requires 'python' on PATH.
    //std::string cmd = "python Python_sample\\Python_sample.py";
    std::string cmd = "python D:\\Study\\git-sample_code\\sample-code\\c++\\cpp_using_python\\Python_sample\\Python_sample.py";
    FILE* pipe = _popen(cmd.c_str(), "r");
    if (!pipe) {
        std::cerr << "Failed to run python subprocess\n";
        return 1;
    }
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::cout << buffer;
    }
    _pclose(pipe);
    return 0;
#endif
}

// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴

// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.
