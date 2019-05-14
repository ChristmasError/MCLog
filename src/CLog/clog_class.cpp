#pragma once 

#include "clog_class.h"

#include <time.h>
#include <stdarg.h>

using namespace std;

CLog::CLog()
{
	CSLock m_csLock;
	m_SplitLines = 0;
	m_LogBufSize = 0;
	m_count = 0;
	m_today = 0;
	m_fp = NULL;
	m_buf = NULL;
	m_isAsync = false;
}
CLog::~CLog()
{
	if (m_fp != NULL)
		fclose(m_fp);
}

void* CLog::AsyncWriteLog()
{
	std::string signle_log;
	while (1)
	{
		printf("%d\n", !m_LogQueue.size());
		if (!m_LogQueue.size())
		{
			CSAutoLock lock(m_csLock);
			signle_log = m_LogQueue.front();
			m_LogQueue.pop_front();
			fputs(signle_log.c_str(), m_fp);//写入日志文件
		}
		else
		{
			CSAutoLock lock(m_csLock);
			signle_log = m_LogQueue.front();
			m_LogQueue.pop_front();
			fputs(signle_log.c_str(), m_fp);
		}
	}
}

DWORD WINAPI CLog::FlushLogThread(LPVOID lpParam)
{
	CLog::LogInstance()->AsyncWriteLog();
	if (lpParam != NULL && lpParam != INVALID_HANDLE_VALUE)
	{
		CloseHandle(lpParam);
		lpParam = NULL;
	}
	return 0;
}

bool CLog::init(const char* file_name, const char* log_path, int log_buf_size, int split_lines, int max_queue_size)
{
	// 创建日志线程
	if (m_LogQueue.size() >= 1)
	{
		m_isAsync = true;
		std::deque<std::string>(max_queue_size);
		HANDLE flushlogthread = CreateThread(NULL, NULL, FlushLogThread,(void*)this, 0, NULL);
		CloseHandle(flushlogthread);
	}


	strcpy(m_LogPath, log_path);
	m_LogBufSize = log_buf_size;
	m_buf = new char[m_LogBufSize];
	memset(m_buf, '\0', sizeof(m_buf));
	m_SplitLines = split_lines;
	time_t t = time(NULL);
	struct tm* sys_tm = localtime(&t);
	const char *p = strrchr(file_name, '/');
	char log_file_address[256] = { 0 };
	if (p == NULL)
	{
		snprintf(m_LogName, 255, "%d_%02d_%02d_%s", sys_tm->tm_year + 1900, sys_tm->tm_mon + 1, sys_tm->tm_mday, file_name);
	}
	else
	{
		strcpy(m_LogName, p + 1);
		snprintf(m_LogName, 255, "%d_%02d_%02d_%s", sys_tm->tm_year + 1900, sys_tm->tm_mon + 1, sys_tm->tm_mday, file_name);
	}
	strcpy(log_file_address, m_LogPath);
	strcat(log_file_address, m_LogName);
	cout << "log_file_address: " << log_file_address << endl;
	m_today = sys_tm->tm_mday;
	m_fp = fopen(log_file_address, "a");
	if (m_fp == NULL)
	{
		std::cerr << "打开日志文件失败!\n";
		return false;
	}
	return true;
}

void CLog::write_log(int level, const char*format, ...)
{
	clock_t start, end;
	start = clock();
	time_t t = time(NULL);
	struct tm* sys_tm = localtime(&t);
	char s[16] = { 0 };
	switch (level)
	{
		case 0:strcpy(s, "[DEBUG]:"); break;
		case 1:strcpy(s, "[INFO]:"); break;
		case 2:strcpy(s, "[WARN]:"); break;
		case 3:strcpy(s, "[ERROR]:"); break;
		default:
			strcpy(s, "[INFO]:"); break;
	}

	CSAutoLock lock(m_csLock);
	m_count++;
	// 对比日志时间与系统时间
	if (m_today != sys_tm->tm_mday || m_count % m_SplitLines == 0)
	{
		char new_log_path[256] = { 0 };
		char new_log_name[256] = { 0 };
		strcpy(new_log_path, m_LogPath);
		fflush(m_fp);
		fclose(m_fp);
		char tail[16] = { 0 };
		snprintf(tail, 16, "%d_%02d_%02d_", sys_tm->tm_year + 1900, sys_tm->tm_mon + 1, sys_tm->tm_mday);

		if (m_today != sys_tm->tm_mday)
		{
			snprintf(new_log_name, 255, "%s%s", tail, m_LogName);
			m_today = sys_tm->tm_mday;
			m_count = 0;
		}
		else
		{
			snprintf(new_log_name, 255, "%s%s.%d", tail, m_LogName , m_count/m_SplitLines);
		}
		strcat(new_log_path, new_log_name);
		m_fp = fopen(new_log_path, "a");
	}
	lock.UnLock();
	va_list valst;
	va_start(valst, format);
	std::string log_str;
	lock.Lock();
	int n = snprintf(m_buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06d %s ", sys_tm->tm_year + 1900, sys_tm->tm_mon + 1, sys_tm->tm_mday, sys_tm->tm_hour, sys_tm->tm_min, sys_tm->tm_sec, start, s);
	int m = vsnprintf(m_buf + n, m_LogBufSize - 1, format, valst);
	m_buf[n + m + 1] = '\n';
	log_str = m_buf;
	lock.UnLock();
	if (m_isAsync) // !m_logQueue->full()
	{
		lock.Lock();
		m_LogQueue.push_back(log_str);
		lock.UnLock();
	}
	else
	{
		lock.Lock();
		fputs(log_str.c_str(), m_fp);
		lock.UnLock();
	}
}

void CLog::flush()
{
	CSAutoLock lock(m_csLock);
	fflush(m_fp);
}