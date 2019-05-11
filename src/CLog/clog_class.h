#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "cs_auto_lock_class.h"

#include <iostream>
#include <string>
#include <stdarg.h>
#include <thread>
#include <deque>

#define DEBUG 0
#define INFO 1
#define WARN 2
#define ERROR 3

class CLog 
{
public:
static CLog* Instance()
{
	static CLog instance;
	return &instance;
}
// 清空日志线程
static DWORD WINAPI FlushLogThread(LPVOID lpParam);
// 初始化
bool init(const char* file_name, int log_buf_size = 1024 * 8, int split_lines = 5000000, int max_queue_size = 0);
// 写日志
void write_log(int level, const char*format, ...);
// 清空
void flush(); 

private:
	CLog();
	~CLog();

	void* AsyncWriteLog();

private:
	CSLock m_csLock;
	char m_dirName[128];
	char m_LogName[128];
	int m_SplitLines;
	int m_LogBufSize;
	long long m_count;
	int m_today;
	FILE *m_fp;
	char *m_buf;
	std::deque<std::string> m_LogQueue;
	bool m_isAsync;
};

#define LOG_DEBUG(format,...) CLog::Instance()->write_log(DEBUG,format,__VA_ARGS__)
#define LOG_INFO(format,...) CLog::Instance()->write_log(INFO,format,__VA_ARGS__)
#define LOG_WARN(format,...) CLog::Instance()->write_log(WARN,format,__VA_ARGS__)
#define LOG_ERROR(format,...) CLog::Instance()->write_log(ERROR,format,__VA_ARGS__)