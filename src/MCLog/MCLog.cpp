#pragma once 

#include "MCLog.h"  //MCLOG_API

MCLog* MCLog::_mInstance = NULL;
uint32_t MCLog::_mPerBufSize = (1024 * 1024); //40Kb

MCLog::MCLog()
{
	_mLogPath = new char[MAX_PATH];
	_mSysDate = new char[11];
	_mLogFileLocation = new char[MAX_PATH];
	memset(_mLogPath, 0, MAX_PATH);
	memset(_mSysDate, 0, 11);
	memset(_mLogFileLocation, 0, MAX_PATH);
	_mFp = NULL;
	_mBufCnt = 5;
	_mLastErrorTime = 0;
	uint32_t a = MEM_USE_LIMIT;
	LogBuffer* head = new LogBuffer(PER_BUFFER_SIZE);
	if (!head)
	{
		std::cerr<< "Error : no space to allocate LogBuffer\n";
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
			std::cerr << "Error : no space to allocate LogBuffer\n";
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
MCLog::~MCLog()
{
	if (_mFp != NULL)
		fclose(_mFp);
	delete _mLogPath;
	CloseHandle(_hWriteFileSemaphore);
	::DeleteCriticalSection(&_hCS_CurBufferLock);
}



void MCLog::SetLogPath(const char* log_path/*=LOG_DEFAULE_PATH*/)
{
	int logpath_size = strlen(log_path);
	memcpy(_mInstance->_mLogPath, log_path, logpath_size + 1);
	if (_mLogPath[logpath_size - 1] != '\\' && _mLogPath[logpath_size] != '/')
		strcat(_mLogPath, "\\");
}

void MCLog::WriteLogCache(const char* log_name, const char* log_str)
{
	GetLocalTime(&_mSys);
	uint32_t year, month, day, hour, minute, second, milli_second;
	year = _mSys.wYear, month = _mSys.wMonth, day = _mSys.wDay;
	hour = _mSys.wHour, minute = _mSys.wMinute, second = _mSys.wSecond, milli_second = _mSys.wMilliseconds;
	
	if (_mLastErrorTime && _mSys.wSecond - _mLastErrorTime < 5)
	{
		Sleep(2000);
		//WriteLogCache(log_name, log_str);
		return;
	}
		
	char log_line[LOG_LEN_LIMIT];
	sprintf(log_line, "%d.%d.%d-%d:%d:%d:%d %s", year, month, day, hour, minute, second, milli_second, log_str);
	if (log_line[strlen(log_line) - 1] != '\n') //日志'\n'的缺省
	{
		strcat(log_line, "\n");
	}
	int logStr_len = strlen(log_line);

	_mLastErrorTime = 0;
	bool tell_back = false;
	::EnterCriticalSection(&_hCS_CurBufferLock);
	if (strlen(_mCurBuffer->mCurLogName) == 0)  //设定缓存区块对应日志文件名
		memcpy(_mCurBuffer->mCurLogName, log_name, strlen(log_name) + 1);

	if (strcmp(_mCurBuffer->mCurLogName, log_name) != 0)   //_mCurBuffer->mCurLogName != log_name
	{
		LogBuffer* ptemp_buf = GetRelevantBuffer(log_name);
		if (ptemp_buf != NULL) //有合适缓存区
		{
			if (ptemp_buf->Empty())
				memcpy(ptemp_buf->mCurLogName, log_name, strlen(log_name) + 1);
			ptemp_buf->AppendLog(log_line, logStr_len);
		}
		else //ptemp_buf == NULL
		{
			if (PER_BUFFER_SIZE * (_mBufCnt + 1) > MEM_USE_LIMIT) //日志文件>=日志最大大小限制
			{
				std::cerr << "Error : no more log space can use\n";
				_mCurBuffer = _mCurBuffer->mNext;
				_mLastErrorTime = _mSys.wSecond;
			}
			else
			{
				/*LogBuffer* prev_buf = _mCurBuffer->mPrev;
				LogBuffer* pnew_buf = new LogBuffer(PER_BUFFER_SIZE);
				pnew_buf->mPrev = prev_buf;
				prev_buf->mNext = pnew_buf;
				pnew_buf->mNext = _mCurBuffer;
				_mCurBuffer->mPrev = pnew_buf;
				_mBufCnt += 1;
				ptemp_buf = pnew_buf;
				std::cerr << _mBufCnt << std::endl;
				memcpy(ptemp_buf->mCurLogName, log_name, strlen(log_name) + 1);
				ptemp_buf->AppendLog(log_line, logStr_len);*/
				LogBuffer* pnext_buf = _mCurBuffer->mNext;
				ptemp_buf = new LogBuffer(PER_BUFFER_SIZE);
				ptemp_buf->mPrev = _mCurBuffer;
				_mCurBuffer->mNext = ptemp_buf;
				ptemp_buf->mNext = pnext_buf;
				pnext_buf->mPrev = ptemp_buf;
				_mBufCnt += 1;
				std::cerr << _mBufCnt << std::endl;
				memcpy(ptemp_buf->mCurLogName, log_name, strlen(log_name) + 1);
				ptemp_buf->AppendLog(log_line, logStr_len);
			}
		}
	}
	else //_mCurBuffer->mCurLogName == log_name
	{
		if (_mCurBuffer->mStatus == LogBuffer::FREE && _mCurBuffer->AvailableLen() >= logStr_len)
		{
			_mCurBuffer->AppendLog(log_line, logStr_len);
		}
		else // (buffer->mStatus = FREE but Buffer->AvailableLen() is not enough) || ( Buffer->mStatus = FULL)
		{
			if (_mCurBuffer->mStatus == LogBuffer::FREE)
			{
				_mCurBuffer->mStatus = LogBuffer::FULL; //set FULL
				LogBuffer* pnext_buf = _mCurBuffer->mNext;
				tell_back = true;

				if (pnext_buf->mStatus == LogBuffer::FULL)
				{
					if (PER_BUFFER_SIZE * (_mBufCnt + 1) > MEM_USE_LIMIT) //日志文件>=日志最大大小限制
					{
						std::cerr << "Error : no more log space can use\n";
						_mCurBuffer = pnext_buf;
						_mLastErrorTime = _mSys.wSecond;
					}
					else
					{
						LogBuffer* pnew_buf = new LogBuffer(PER_BUFFER_SIZE);
						pnew_buf->mPrev = _mCurBuffer;
						_mCurBuffer->mNext = pnew_buf;
						pnew_buf->mNext = pnext_buf;		
						pnext_buf->mPrev = pnew_buf;
						_mCurBuffer = pnew_buf;
						_mBufCnt += 1;
						std::cerr << _mBufCnt << std::endl;
						memcpy(_mCurBuffer->mCurLogName, log_name, strlen(log_name) + 1);
					}
				}
				else
				{
					_mCurBuffer = pnext_buf;
				}

				if (!_mLastErrorTime)
				{
					_mCurBuffer->AppendLog(log_line, logStr_len);
				}
			}
			else
			{
				_mLastErrorTime = _mSys.wSecond;
			}
		}
	}
	::LeaveCriticalSection(&_hCS_CurBufferLock);
 
	if (tell_back)
	{
		ReleaseSemaphore(_hWriteFileSemaphore, 1, NULL);  //唤醒消费者线程
	}
}

LogBuffer* MCLog::GetRelevantBuffer(const char* log_name)
{
	LogBuffer* temp_next = _mCurBuffer->mNext;
	LogBuffer* temp_curBuf = _mCurBuffer;
	bool isfind = false;
	while (temp_next != temp_curBuf)
	{
		if (strcmp(temp_next->mCurLogName, log_name) == 0 && temp_next->AvailableLen() > LOG_LEN_LIMIT && temp_next->mStatus == LogBuffer::FREE)
		{
			isfind = true;
			break;
		}
		else if (temp_next->Empty())
		{
			isfind = true;
			break;
		}
		temp_next = temp_next->mNext;
	}
	if (isfind)
		return temp_next;
	else
		return NULL; //双向链表无符合缓存区
}

////////////////////////////////////////////////////////////////////////////////////////////
//                                 消费线程相关函数										  //	
////////////////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI MCLog::CachePersistThreadFunc(LPVOID lpParam)
{
#ifdef  _DEBUG
	std::cerr << "Start Write Log File.\n";
#endif //  DEBUG
	MCLog::LogInstance()->BufferPersist();
	return NULL;
}

void MCLog::BufferPersist()
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
			continue;
		}
		if (_mPrstBuffer->mStatus == LogBuffer::FREE)
		{
			::EnterCriticalSection(&_hCS_CurBufferLock);
			assert(_mPrstBuffer == _mCurBuffer);    //to test
			_mCurBuffer->mStatus = LogBuffer::FULL;
			_mCurBuffer = _mCurBuffer->mNext;
			::LeaveCriticalSection(&_hCS_CurBufferLock);	
		}
		if (!OpenFile(_mPrstBuffer->mCurLogName))
		{
			std::cerr << "Error : open log file failed.\n";
			continue;
		}

		_mPrstBuffer->WriteFile(_mFp);
		fflush(_mFp);
		::EnterCriticalSection(&_hCS_CurBufferLock);
		_mPrstBuffer->Clear();
		_mPrstBuffer =_mPrstBuffer->mNext;
		::LeaveCriticalSection(&_hCS_CurBufferLock);
	}
	std::cerr << "Persist thread quit\n";
}



bool MCLog::OpenFile(const char* log_name)
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
		return _mFp != NULL;
}

void MCLog::GetSystemDate(char* log_date)
{
	GetLocalTime(&_mSys);
	sprintf(log_date, "%d-%d-%d", _mSys.wYear, _mSys.wMonth, _mSys.wDay);
	return;
}


bool MCLog::CreateFilePath(const char* log_path)
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