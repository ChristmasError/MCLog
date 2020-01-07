#include <iostream>
#include <windows.h>
#include <ctime>
#include "MCLog.h"

using namespace std;

static const int gThreadsNum = 10;
HANDLE hAllThreadsFinish = CreateSemaphore(NULL, 0, gThreadsNum, NULL);
HANDLE hABLogFinish = CreateSemaphore(NULL, 0, 20, NULL);

DWORD WINAPI writeLog(LPVOID lpParam)
{
	for (int i = 0; i < 1000; i++)
	{
		WRITE_LOG("test10threads.txt", "111111111111111111111111111111111111111111111111111111111111111111111111111111\n");
	}
	ReleaseSemaphore(hAllThreadsFinish, 1, NULL);
	return 0;
}

DWORD WINAPI writeLogA(LPVOID lpParam)
{
	for (int i = 0; i < 10000; i++)
	{
		WRITE_LOG("testa.txt", "aaaaaaaaaaaaaa");
		//WRITE_LOG("testb.txt", "bbbbbbbbbbbbbb\n");
	}
	ReleaseSemaphore(hABLogFinish, 1, NULL);
	return 0;
}

DWORD WINAPI writeLogB(LPVOID lpParam)
{
	for (int i = 0; i < 10000; i++)
	{
		WRITE_LOG("testa.txt", "AAAAAAAAAAAAAA\n");
		//WRITE_LOG("testb.txt", "BBBBBBBBBBBBBB\n");
	}
	ReleaseSemaphore(hABLogFinish, 1, NULL);
	return 0;
}

int main()
{

	int i = 10;
	LOG_INIT();
	//SET_LOGPATH(".\\Log_temp");
	Sleep(3000);
	clock_t start = clock();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*while(i--)
	{ 
		WRITE_LOG("test1.txt", "111111111111\n");
		WRITE_LOG("test2.txt", "222222222222\n");
		WRITE_LOG("test3.txt", "333333333333\n");
		WRITE_LOG("test4.txt", "444444444444\n");
	}
	i = 10;
	cerr << "first round finish\n";
	Sleep(3000);
	while (i--)
	{
		WRITE_LOG("test1.txt", "xxxxxxxxxxxxxx\n");
	 	WRITE_LOG("test2.txt", "xxxxxxxxxxxxxx\n");
	}
	i = 1000;
	cerr << "second round finish\n";
	Sleep(3000);
	while (i--)
	{
		WRITE_LOG("test1.txt", "111111111111\n");
	}
	i = 10;
	cerr << "third round finish\n";
	Sleep(3000);
	while (i--)
	{
		WRITE_LOG("test3.txt", "333333333333\n");
		WRITE_LOG("test4.txt", "444444444444\n");
	}
	cerr << "all round finish\n";
	clock_t end = clock();
	cerr << "used time: " << (double)(end - start) / CLOCKS_PER_SEC << "seconds" << endl;*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/*for (int i = 0; i < gThreadsNum; i++)
	{
		HANDLE writelogthread = CreateThread(NULL, NULL, writeLog, NULL, 0, NULL);
		if(writelogthread!=0)
			CloseHandle(writelogthread);
	}
	WaitForSingleObject(hAllThreadsFinish,INFINITE);
	clock_t end = clock();
	cerr << "10 threads 1000 times 100 byte logs used time: " << (double)(end - start) / CLOCKS_PER_SEC << "seconds" << endl;*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	for (int i = 0; i < gThreadsNum; i++)
	{
		HANDLE writelogthread = CreateThread(NULL, NULL, writeLogA, NULL, 0, NULL);
		if(writelogthread!=0)
			CloseHandle(writelogthread);
	}
	for (int i = 0; i < gThreadsNum; i++)
	{
		HANDLE writelogthread = CreateThread(NULL, NULL, writeLogB, NULL, 0, NULL);
		if (writelogthread != 0)
			CloseHandle(writelogthread);
	}
	/*HANDLE ThreadA = CreateThread(NULL, NULL, writeLogA, NULL, 0, NULL);
	HANDLE ThreadB = CreateThread(NULL, NULL, writeLogB, NULL, 0, NULL);*/
	WaitForSingleObject(hABLogFinish, INFINITE);
	clock_t end = clock();
	cerr << "10 threads 1000 times 100 byte logs used time: " << (double)(end - start) / CLOCKS_PER_SEC << "seconds" << endl; 

	cin.get();
	return 0;
}