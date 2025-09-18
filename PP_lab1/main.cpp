#include "tchar.h"
#include <iostream>
#include <cstdio>
#include <string>
#include <windows.h>

DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
	std::cout << "Поток №" << (intptr_t)lpParam << std::endl;

	ExitThread(0); // функция устанавливает код завершения потока в 0
}
int _tmain(int argc, _TCHAR* argv[])
{
	const int threadCount = std::stoi(argv[1]);
	HANDLE* handles = new HANDLE[threadCount];

	for (int i = 0; i < threadCount; i++)
	{
		handles[i] = CreateThread(NULL, 0, &ThreadProc, LPVOID(i + 1), CREATE_SUSPENDED, NULL);
	}

	for (int i = 0; i < threadCount; i++)
	{
		ResumeThread(handles[i]);
	}

	// ожидание окончания работы двух потоков
	WaitForMultipleObjects(threadCount, handles, true, INFINITE);

	delete[] handles;

	return 0;
}