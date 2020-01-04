#pragma once 

#include <io.h>     //access函数的头文件
#include <stdarg.h>
#include <direct.h>
#include <assert.h>
#ifdef  _DEBUG
#include <iostream>
#endif //  DEBUG

#include "./CLog.h"

using namespace std;

CLog* CLog::_mInstance = NULL;
uint32_t CLog::_mPerBufSize = (1024 * 1024);//1Mb

CLog::CLog()
{
	_mLogPath    = new char[MAX_PATH];
	_mSysDate	 = new char[11];
	_mLogFileLocation = new char[MAX_PATH];
	memset(_mLogPath, 0, MAX_PATH);
	memset(_mSysDate, 0, 11);
	memset(_mLogFileLocation, 0, MAX_PATH);
	_mFp          = NULL;
	_mBufCnt = 3;
	uint32_t a = MEM_USE_LIMIT;
	LogBuffer* head = new LogBuffer(PER_BUFFER_SIZE);
	if (!head)
	{
		fprintf(stderr, "Error : no space to allocate LogBuffer\n");
		exit(1);
	}
	//初始化缓存区块-双向链表
	LogBuffer* cur_buf = NULL;
	LogBuffer* prev_buf = head;
	for (int i = 1; i < _mBufCnt; ++i)
	{
		cur_buf = new LogBuffer(PER_BUFFER_SIZE);
		if (!cur_buf)
		{
			fprintf(stderr, "Error : no space to allocate LogBuffer\n");
			exit(1);
		}
		cur_buf->mPrev = prev_buf;
		prev_buf->mNext = cur_buf;
		prev_buf = cur_buf;
	}
	prev_buf->mNext = head;
	head->mPrev = prev_buf;
	_mCurBuffer = head;
	_mPrstBuffer = head;

	_mPid = getpid();
	
}
CLog::~CLog()
{
	if (_mFp != NULL)
		fclose(_mFp);
	delete _mLogPath;
	CloseHandle(_hWriteFileSemaphore);
	CloseHandle(_hMutex);
}

LogBuffer* CLog::GetFullBuffer()
{
	LogBuffer* pbuffer = _mPrstBuffer->mNext;
	while (pbuffer != _mPrstBuffer)
	{
		if (pbuffer->mStatus == LogBuffer::FULL)
			break;
		else
			pbuffer = pbuffer->mNext;
	}
	if (pbuffer == NULL)
		pbuffer = _mCurBuffer;
	return pbuffer;
}

void CLog::SetLogPath(const char* log_path/*=LOG_DEFAULE_PATH*/)
{
	int logpath_size = strlen(log_path);
	memcpy(_mInstance->_mLogPath, log_path, logpath_size + 1);
	if (_mLogPath[logpath_size -1] != '\\' && _mLogPath[logpath_size] != '/')
		strcat(_mLogPath, "\\");
}

void CLog::WriteLogCache(const char* log_name, const char* log_str)
{
	GetLocalTime(&_mSys);
	uint32_t year,month,day,hour, minute, second,m_second;
	year = _mSys.wYear, month = _mSys.wMonth, day = _mSys.wDay;
	hour = _mSys.wHour, minute = _mSys.wMinute, second = _mSys.wSecond, m_second = _mSys.wMilliseconds;
	char log_line[LOG_LEN_LIMIT];
	sprintf(log_line, "%d.%d.%d-%d:%d:%d:%d %s",year,month,day, hour, minute, second , m_second, log_str);
	int logStr_len = strlen(log_line);

	_mErrortime = 0;
	bool tell_back = false;

	WaitForSingleObject(_hMutex, INFINITE);
	if (strlen(_mCurBuffer->mCurLogName) == 0)  //设定缓存区块对应日志文件名
		memcpy(_mCurBuffer->mCurLogName, log_name, strlen(log_name) + 1);
	//if(strstr(_mCurBuffer->mCurLogName, ".txt") == NULL)  //".txt"的缺省
	//	strcat(_mCurBuffer->mCurLogName, ".txt");
	if (strcmp(_mCurBuffer->mCurLogName, log_name) != 0)   //对新的日志文件进行写入
	{ 
		_mCurBuffer->mStatus = LogBuffer::FULL;
		tell_back = true;
		LogBuffer* pnext_buf = _mCurBuffer->mNext;
		if (pnext_buf->Empty())
		{
			_mCurBuffer = pnext_buf;
			memcpy(_mCurBuffer->mCurLogName, log_name, strlen(log_name) + 1);
		}
		else
		{
			if (PER_BUFFER_SIZE * (_mBufCnt + 1) > MEM_USE_LIMIT) //日志文件>=日志最大大小限制
			{
				fprintf(stderr, "Error : no more log space can use\n");
				_mCurBuffer = pnext_buf;
				_mErrortime = _mSys.wSecond;
			}
			else 
			{
				LogBuffer* pnew_buf = new LogBuffer(PER_BUFFER_SIZE);
				pnew_buf->mPrev = _mCurBuffer;
				pnew_buf->mNext = pnext_buf;
				_mCurBuffer->mNext = pnew_buf;
				pnext_buf->mPrev = pnew_buf;
				_mCurBuffer = pnew_buf;
				_mBufCnt += 1;
				memcpy(_mCurBuffer->mCurLogName, log_name, strlen(log_name) + 1);
			}		
		}
		_mCurBuffer->AppendLog(log_line, logStr_len);
	}
	else
	{
		if (_mCurBuffer->mStatus == LogBuffer::FREE && _mCurBuffer->AvailableLen() >= logStr_len)
		{
			_mCurBuffer->AppendLog(log_line, logStr_len);
		}
		else
		{
			//1. _mCurBuffer->mStatus = LogBuffer::FREE but _mCurBuffer->AvailableLen() < mPrev_len
			//2. _mCurBuffer->mStatus = LogBuffer::FULL
			if (_mCurBuffer->mStatus == LogBuffer::FREE)
			{
				_mCurBuffer->mStatus = LogBuffer::FULL; //set FULL
				LogBuffer* pnext_buf = _mCurBuffer->mNext;
				tell_back = true;

				if (pnext_buf->mStatus == LogBuffer::FULL)
				{
					if (PER_BUFFER_SIZE * (_mBufCnt + 1) > MEM_USE_LIMIT) //日志文件>=日志最大大小限制
					{
						fprintf(stderr, "Error : no more log space can use\n");
						_mCurBuffer = pnext_buf;
						_mErrortime = _mSys.wSecond;
					}
					else
					{
						LogBuffer* pnew_buf = new LogBuffer(PER_BUFFER_SIZE);
						pnew_buf->mPrev = _mCurBuffer;
						pnew_buf->mNext = pnext_buf;
						_mCurBuffer->mNext = pnew_buf;
						pnext_buf->mPrev = pnew_buf;
						_mCurBuffer = pnew_buf;
						_mBufCnt += 1;
						memcpy(_mCurBuffer->mCurLogName, log_name, strlen(log_name) + 1);
					}
				}
				else
				{
					_mCurBuffer = pnext_buf;
				}

				if (!_mErrortime)
				{
					_mCurBuffer->AppendLog(log_line, logStr_len);
				}
			}
			else
			{
				_mErrortime = _mSys.wSecond;
			}
		}
	}
	ReleaseMutex(_hMutex);
	if (tell_back)
	{
		ReleaseSemaphore(_hWriteFileSemaphore, 1, NULL);  //唤醒消费者线程
	}
}


////////////////////////////////////////////////////////////////////////////////////////////
//                                 消费线程相关函数										  //	
////////////////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI CLog::CachePersistThreadFunc(LPVOID lpParam)
{
#ifdef  _DEBUG
	cerr << "Start Write Log File.\n";
#endif //  DEBUG
	CLog::CLogInstance()->CachePersist();
	return NULL;
}

void CLog::CachePersist()
{
	while (true)
	{
		DWORD dwRet = 0;
		if (_mPrstBuffer->mStatus == LogBuffer::FREE)
		{
			dwRet = WaitForSingleObject(_hWriteFileSemaphore, BUFF_WAIT_TIME); //等待BUFF_WAIT_TIME，或者被唤醒
		}
		if (_mPrstBuffer->Empty())
		{
			_mPrstBuffer = _mCurBuffer;
			continue;
		}
		if (_mPrstBuffer->mStatus == LogBuffer::FREE)
		{
			WaitForSingleObject(_hMutex, INFINITE);
			assert(_mCurBuffer == _mPrstBuffer);    //to test
			_mCurBuffer->mStatus = LogBuffer::FULL;
			_mCurBuffer = _mCurBuffer->mNext;
			ReleaseMutex(_hMutex);
		}

		if(!OpenFile(_mPrstBuffer->mCurLogName))
		{
			std::cerr << "Error : open log file failed.\n";
		}
		//写日志
#ifdef _DEBUG
		cerr << "[info] write\n";
#endif // _DEBUG
		_mPrstBuffer->WriteFile(_mFp);
		fflush(_mFp);

		_mPrstBuffer->Clear();
		cerr << _mBufCnt << endl;
		_mPrstBuffer = GetFullBuffer();//_mPrstBuffer->mNext;
	}
	cerr << "Consumer thread quit\n";
}

bool CLog::OpenFile(const char* log_name)
{
	GetSystemDate(_mSysDate);
	if (strlen(_mLogPath) + strlen(log_name) >= MAX_PATH - 10)
	{
		std::cerr << "Error : log file path is too long.\n";
		return false;
	}
	char fileLocation[MAX_PATH];
	sprintf(fileLocation, "%s%s\\%s", _mLogPath, _mSysDate, log_name);
	if (strcmp(_mLogFileLocation, fileLocation) != 0)
	{
		if (_mFp != NULL)
			fclose(_mFp);
		memcpy(_mLogFileLocation, fileLocation, strlen(fileLocation) + 1);
		CreateFilePath(_mLogFileLocation);
		_mFp = fopen(_mLogFileLocation, "a");
		return _mFp != NULL;
	}
	else
		return true;
}

void CLog::GetSystemDate(char*log_date)
{
	GetLocalTime(&_mSys);
	sprintf(log_date, "%d-%d-%d", _mSys.wYear, _mSys.wMonth, _mSys.wDay);
	return;
}


bool CLog::CreateFilePath(const char* log_path)
{
	int dirPathLen = strlen(log_path);
	if (dirPathLen > MAX_PATH)
	{
		return false;
	}
	char tmpDirPath[MAX_PATH] = { 0 };
	for (uint32_t i = 0; i < dirPathLen; ++i)
	{
		tmpDirPath[i] = log_path[i];
		if (tmpDirPath[i] == '\\' || tmpDirPath[i] == '/')
		{
			if (access(tmpDirPath, 0) != 0) //判断文件是否存在
			{
				if (mkdir(tmpDirPath) != 0)
				{
					std::cerr << "Error : create new log directory failed.\n";
					return false;
				}
			}
		}
	}
	return true;
}