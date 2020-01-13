#include <iostream>
#include <windows.h>
#include <ctime>
#include "MCLog.h"

using namespace std;

static const int gThreadsNum = 10;
HANDLE hAllThreadsFinish = CreateSemaphore(NULL, 0, gThreadsNum, NULL);
HANDLE hABLogFinish = CreateSemaphore(NULL, 0, gThreadsNum*2, NULL);

DWORD WINAPI writeLog(LPVOID lpParam)
{
	for (int i = 0; i < 10000; i++)
	{
		WRITE_LOG("test10threadsa.txt", "111111111111111111111111111111111111111111111111111111111111111111111111111111\n");
		WRITE_LOG("test10threadsb.txt", "222222222222222222222222222222222222222222222222222222222222222222222222222222\n");
	}
	ReleaseSemaphore(hAllThreadsFinish, 1, NULL);
	return 0;
}

DWORD WINAPI writeLogA(LPVOID lpParam)
{
	for (int i = 0; i < 10000; i++)
	{
		WRITE_LOG("testa.txt", "aaaaaaaaaaaaaa");
		WRITE_LOG("testb.txt", "bbbbbbbbbbbbbb");
	}
	ReleaseSemaphore(hABLogFinish, 1, NULL);
	return 0;
}

DWORD WINAPI writeLogB(LPVOID lpParam)
{
	for (int i = 0; i < 10000; i++)
	{
		WRITE_LOG("testa.txt", "AAAAAAAAAAAAAA\n");
		WRITE_LOG("testb.txt", "BBBBBBBBBBBBBB\n");
	}
	ReleaseSemaphore(hABLogFinish, 1, NULL);
	return 0;
}

HANDLE hTenLogFinish = CreateSemaphore(NULL, 0, 10, NULL);
DWORD WINAPI writeLog1(LPVOID lpParam)
{
	for (int i = 0; i < 100000; i++)
	{
		WRITE_LOG("test1.txt", "AAAAAAAAAAAAAA\n");
		//Sleep(5);
	}
	ReleaseSemaphore(hTenLogFinish, 1, NULL);
	return 0;
}
DWORD WINAPI writeLog2(LPVOID lpParam)
{
	for (int i = 0; i < 100000; i++)
	{
		WRITE_LOG("test2.txt", "AAAAAAAAAAAAAA\n");
		//Sleep(5);

	}
	ReleaseSemaphore(hTenLogFinish, 1, NULL);
	return 0;
}
DWORD WINAPI writeLog3(LPVOID lpParam)
{
	for (int i = 0; i < 100000; i++)
	{
		WRITE_LOG("test3.txt", "AAAAAAAAAAAAAA\n");
		//Sleep(5);

	}
	ReleaseSemaphore(hTenLogFinish, 1, NULL);
	return 0;
}
DWORD WINAPI writeLog4(LPVOID lpParam)
{
	for (int i = 0; i < 100000; i++)
	{
		WRITE_LOG("test4.txt", "AAAAAAAAAAAAAA\n");
		//Sleep(5);

	}
	ReleaseSemaphore(hTenLogFinish, 1, NULL);
	return 0;
}
DWORD WINAPI writeLog5(LPVOID lpParam)
{
	for (int i = 0; i < 100000; i++)
	{
		WRITE_LOG("test5.txt", "AAAAAAAAAAAAAA\n");
		//Sleep(5);

	}
	ReleaseSemaphore(hTenLogFinish, 1, NULL);
	return 0;
}
DWORD WINAPI writeLog6(LPVOID lpParam)
{
	for (int i = 0; i < 100000; i++)
	{
		WRITE_LOG("test6.txt", "AAAAAAAAAAAAAA\n");
		//Sleep(5);

	}
	ReleaseSemaphore(hTenLogFinish, 1, NULL);
	return 0;
}
DWORD WINAPI writeLog7(LPVOID lpParam)
{
	for (int i = 0; i < 100000; i++)
	{
		WRITE_LOG("test7.txt", "AAAAAAAAAAAAAA\n");
		//Sleep(5);

	}
	ReleaseSemaphore(hTenLogFinish, 1, NULL);
	return 0;
}
DWORD WINAPI writeLog8(LPVOID lpParam)
{
	for (int i = 0; i < 100000; i++)
	{
		WRITE_LOG("test8.txt", "AAAAAAAAAAAAAA\n");
		//Sleep(5);

	}
	ReleaseSemaphore(hTenLogFinish, 1, NULL);
	return 0;
}
DWORD WINAPI writeLog9(LPVOID lpParam)
{
	for (int i = 0; i < 100000; i++)
	{
		WRITE_LOG("test9.txt", "AAAAAAAAAAAAAA\n");
		//Sleep(5);

	}
	ReleaseSemaphore(hTenLogFinish, 1, NULL);
	return 0;
}
DWORD WINAPI writeLog10(LPVOID lpParam)
{
	for (int i = 0; i < 100000; i++)
	{
		WRITE_LOG("test10.txt", "AAAAAAAAAAAAAA\n");
		//Sleep(5);

	}
	ReleaseSemaphore(hTenLogFinish, 1, NULL);
	return 0;
}

int main()
{
	LOG_INIT();
	//SET_LOGPATH(".\\Log_temp");
	Sleep(3000);
	clock_t start = clock();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*int i = 10;
	while(i--)
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
	//
	//for (int i = 0; i < gThreadsNum; i++)
	//{
	//	HANDLE writelogthread = CreateThread(NULL, NULL, writeLog, NULL, 0, NULL);
	//	if(writelogthread!=0)
	//		CloseHandle(writelogthread);
	//}
	//WaitForSingleObject(hAllThreadsFinish,INFINITE);
	//clock_t end = clock();
	//cerr << "10 threads 1000 times 100 byte logs used time: " << (double)(end - start) / CLOCKS_PER_SEC << "seconds" << endl;

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
	WaitForSingleObject(hABLogFinish, INFINITE);
	clock_t end = clock();
	cerr << "20 threads 1000 times 100 byte logs used time: " << (double)(end - start) / CLOCKS_PER_SEC << "seconds" << endl; 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*HANDLE Thread1 = CreateThread(NULL, NULL, writeLog1, NULL, 0, NULL);
	HANDLE Thread2 = CreateThread(NULL, NULL, writeLog2, NULL, 0, NULL);
	HANDLE Thread3 = CreateThread(NULL, NULL, writeLog3, NULL, 0, NULL);
	HANDLE Thread4 = CreateThread(NULL, NULL, writeLog4, NULL, 0, NULL);
	HANDLE Thread5 = CreateThread(NULL, NULL, writeLog5, NULL, 0, NULL);
	HANDLE Thread6 = CreateThread(NULL, NULL, writeLog6, NULL, 0, NULL);
	HANDLE Thread7 = CreateThread(NULL, NULL, writeLog7, NULL, 0, NULL);
	HANDLE Thread8 = CreateThread(NULL, NULL, writeLog8, NULL, 0, NULL);
	HANDLE Thread9 = CreateThread(NULL, NULL, writeLog9, NULL, 0, NULL);
	HANDLE Thread10 = CreateThread(NULL, NULL, writeLog10, NULL, 0, NULL);
	WaitForSingleObject(hTenLogFinish, INFINITE);
	clock_t end = clock();
	cerr << "20 threads 1000 times 100 byte logs used time: " << (double)(end - start) / CLOCKS_PER_SEC << "seconds" << endl;*/
	/*cin.get();
	 Thread1 = CreateThread(NULL, NULL, writeLog1, NULL, 0, NULL);
	 Thread2 = CreateThread(NULL, NULL, writeLog2, NULL, 0, NULL);
	 Thread3 = CreateThread(NULL, NULL, writeLog3, NULL, 0, NULL);
	 Thread4 = CreateThread(NULL, NULL, writeLog4, NULL, 0, NULL);
	 Thread5 = CreateThread(NULL, NULL, writeLog5, NULL, 0, NULL);
	 Thread6 = CreateThread(NULL, NULL, writeLog6, NULL, 0, NULL);
	 Thread7 = CreateThread(NULL, NULL, writeLog7, NULL, 0, NULL);
	 Thread8 = CreateThread(NULL, NULL, writeLog8, NULL, 0, NULL);
	 Thread9 = CreateThread(NULL, NULL, writeLog9, NULL, 0, NULL);
	 Thread10 = CreateThread(NULL, NULL, writeLog10, NULL, 0, NULL);
	WaitForSingleObject(hTenLogFinish, INFINITE);
	 end = clock();
	cerr << "20 threads 1000 times 100 byte logs used time: " << (double)(end - start) / CLOCKS_PER_SEC << "seconds" << endl;*/

	cin.get();
	return 0;
}