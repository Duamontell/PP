#include <Windows.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "tchar.h"
#include "timeapi.h"

constexpr int OPERATION_COUNT = 20000000	;
constexpr int MEASUREMENTS_COUNT = 20;

struct ThreadArgs
{
	int id;
	DWORD startTime;
};

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
	const auto *args = static_cast<ThreadArgs *>(lpParam);
	std::string marks;

	for (size_t i = 0; i <= OPERATION_COUNT; ++i)
	{
		const DWORD now = timeGetTime();
		const DWORD diff = now - args->startTime;

		if (i % (OPERATION_COUNT / MEASUREMENTS_COUNT) == 0)
		{
			marks += std::to_string(args->id) + "|" + std::to_string(diff) + "\n";
		}
	}

	std::string fileName = "output_" + std::to_string(args->id) + ".txt";
	std::ofstream file(fileName, std::ios::out | std::ios::trunc);
	file << marks << "\n";
	file.close();

	ExitThread(0);
}

int _tmain(int argc, _TCHAR *argv[])
{
	std::string sleep;
	std::cin >> sleep;

	const DWORD start = timeGetTime();

	const int threadCount = std::stoi(argv[1]);
	auto *handles = new HANDLE[threadCount];
	std::vector<ThreadArgs> args(threadCount);

	for (int i = 0; i < threadCount; i++)
	{
		args[i].id = i + 1;
		args[i].startTime = start;

		handles[i] = CreateThread(NULL, 0, &ThreadProc, &args[i], CREATE_SUSPENDED, NULL);
	}

	SetThreadPriority(handles[1], THREAD_PRIORITY_HIGHEST);

	for (int i = 0; i < threadCount; i++)
	{
		ResumeThread(handles[i]);
	}

	WaitForMultipleObjects(threadCount, handles, true, INFINITE);

	delete[] handles;

	return 0;
}
