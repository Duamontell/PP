#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include "tchar.h"

HANDLE hShared;

int ReadFromFile()
{

	std::fstream myfile("balance.txt", std::ios_base::in);
	int result;
	myfile >> result;
	myfile.close();

	return result;
}

void WriteToFile(const int data)
{
	std::fstream myfile("balance.txt",  std::ios_base::out);
	myfile << data << std::endl;
	myfile.close();
}

int GetBalance()
{
	int balance = ReadFromFile();
	return balance;
}

void Deposit(const int money)
{
	int balance = GetBalance();
	balance += money;

	WriteToFile(balance);
	printf("Balance after deposit: %d\n", balance);
}

void Withdraw(const int money)
{
	if (GetBalance() < money)
	{
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
	WaitForSingleObject(hShared, INFINITE);
	Deposit(money);
	ReleaseMutex(hShared);
	ExitThread(0);
}

DWORD WINAPI DoWithdraw(CONST LPVOID lpParameter)
{
	const int money = static_cast<int>(reinterpret_cast<intptr_t>(lpParameter));
	WaitForSingleObject(hShared, INFINITE);
	Withdraw(money);
	ReleaseMutex(hShared);
	ExitThread(0);
}

int _tmain(int argc, _TCHAR* argv[])
{
	constexpr int threadCount = 50;
	std::vector<HANDLE> threadHandles(threadCount);

	hShared = CreateMutex(NULL, FALSE, reinterpret_cast<LPCSTR>(L"Global\\balance"));

	WriteToFile(0);

	SetProcessAffinityMask(GetCurrentProcess(), 1);
	for ( int i = 0; i < threadCount; i++ )
	{
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

	CloseHandle(hShared);

	return 0;
}
