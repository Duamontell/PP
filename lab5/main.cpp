#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include "tchar.h"

CRITICAL_SECTION FileLockingCriticalSection;

int ReadFromFile()
{
	std::fstream myfile("balance.txt", std::ios_base::in);
	int result;
	myfile >> result;
	std::cout << "tec: " << result << std::endl;
	myfile.close();

	return result;
}

void WriteToFile( int data )
{
	std::fstream myfile("balance.txt",  std::ios_base::out);
	myfile << data << std::endl;
	myfile.close();
}

int GetBalance(){
	int balance = ReadFromFile();
	return balance;
}

void Deposit(const int money){
	int balance = GetBalance();
	balance += money;

	WriteToFile(balance);
	printf("Balance after deposit: %d\n", balance);
}

void Withdraw(const int money){
	if (GetBalance() < money){
		printf("Cannot withdraw money, balance lower than %d\n", money);
		return;
	}

	Sleep(20);

	int balance = GetBalance();
	balance -= money;
	WriteToFile(balance);
	printf("Balance after withdraw: %d\n", balance);
}

DWORD WINAPI DoDeposit(CONST LPVOID lpParameter)
{
	const int money = static_cast<int>(reinterpret_cast<intptr_t>(lpParameter));
	EnterCriticalSection(&FileLockingCriticalSection);
	Deposit(money);
	LeaveCriticalSection(&FileLockingCriticalSection);
	ExitThread(0);
}

DWORD WINAPI DoWithdraw(CONST LPVOID lpParameter)
{
	const int money = static_cast<int>(reinterpret_cast<intptr_t>(lpParameter));
	EnterCriticalSection(&FileLockingCriticalSection);
	Withdraw(money);
	LeaveCriticalSection(&FileLockingCriticalSection);
	ExitThread(0);
}

int _tmain(int argc, _TCHAR* argv[])
{
	constexpr int threadCount = 50;
	std::vector<HANDLE> threadHandles(threadCount);

	InitializeCriticalSection(&FileLockingCriticalSection);

	WriteToFile(0);

	SetProcessAffinityMask(GetCurrentProcess(), 1);
	for ( int i = 0; i < 50; i++ ){
		threadHandles[i] = ( i % 2 == 0 )
			? CreateThread(NULL, 0, &DoDeposit, reinterpret_cast<LPVOID>(static_cast<intptr_t>(230)), CREATE_SUSPENDED, NULL)
			: CreateThread(NULL, 0, &DoWithdraw, reinterpret_cast<LPVOID>(static_cast<intptr_t>(1000)), CREATE_SUSPENDED, NULL);
		ResumeThread(threadHandles[i]);
	}

	WaitForMultipleObjects(threadCount, threadHandles.data(), true, INFINITE);
	for (const auto & thread : threadHandles)
	{
		CloseHandle(thread);
	}

	printf("Final Balance: %d\n", GetBalance());

	getchar();

    DeleteCriticalSection(&FileLockingCriticalSection);

	return 0;
}
