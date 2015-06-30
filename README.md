# Logger
A logging utility written in C++ for Windows applications.

How to use it shown in example code in Logger.cpp, but here is a quick view of it:

```cpp
// Logger.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <conio.h>
#include "Logger.h"


using namespace std;

Logger::CStdOutLogContext g_logCtx;

void foo()
{
	// windows sleep
	Sleep(1024);
}

void runTests()
{
	Logger::CLog log(g_logCtx, L"runTests");

	log << L"Logging a size_t: " << 20 << endl;

	string str = "C++ is fun!";
	log << L"Logging a string: " << str << endl;
	log << L"Logging a char*: " << str.c_str() << endl;

	wstring sMessage = L"Hello World!";
	log << L"Logging a wstring: " << sMessage << endl;
	log << L"Logging a wchar_t*: " << sMessage.c_str() << endl;

	vector<unsigned char> vBytes = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!' };
	log << L"Logging a vector<unsigned char>: " << vBytes << endl;
	log << L"Logging a pbData with cbData: " << &vBytes[0] << vBytes.size() << endl;

	Logger::CTimeDiff td;	// or can keep calling td.start() on the same td object
	// call foo()
	log << L"Calling function foo()... " << endl;
	foo();
	td.end();
	log << L"Function foo() took " << td.diff() << L" milliseconds" << endl;

	// logging a window's error
	log << L"Log a Windows's error code with its value: " << Logger::CUtils::GetWin32ErrorString(ERROR_EFS_ALG_BLOB_TOO_BIG) << endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	runTests();
	_getch();
	return 0;
}
```
