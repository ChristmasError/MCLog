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
	_mBufCnt = 3;
	_mLastErrorTime = 0;
	uint32_t a = MEM_USE_LIMIT;
	LogBuffer* head = new LogBuffer(PER_BUFFER_SIZE);
	if (!head)
	{
		std::cerr<< "Error : no space to allocate LogBuffer\n";
		exit(1);
	}
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
	_mSecBuffer = NULL;

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

void MCLog::LogWriteBuffer(const char* log_name, const char* log_str)
{
	GetLocalTime(&_mSys);
	uint32_t year, month, day, hour, minute, second, milli_second;
	year = _mSys.wYear, month = _mSys.wMonth, day = _mSys.wDay;
	hour = _mSys.wHour, minute = _mSys.wMinute, second = _mSys.wSecond, milli_second = _mSys.wMilliseconds;
	
	if (_mLastErrorTime && _mSys.wSecond - _mLastErrorTime < RELOG_TIME_THRESOLD)
	{
		return;
	}
		
	char log_line[PER_LOG_LEN_LIMIT];
	sprintf(log_line, "%d.%d.%d-%d:%d:%d:%d %s", year, month, day, hour, minute, second, milli_second, log_str);
	if (log_line[strlen(log_line) - 1] != '\n') //log '\n' default
	{
		strcat(log_line, "\n");
	}
	int logStr_len = strlen(log_line);

	_mLastErrorTime = 0;
	bool tell_back = false;
	::EnterCriticalSection(&_hCS_CurBufferLock);

	if (strlen(_mCurBuffer->mCurLogName) == 0)  
		memcpy(_mCurBuffer->mCurLogName, log_name, strlen(log_name) + 1);

	if (strcmp(_mCurBuffer->mCurLogName, log_name) != 0)   //_mCurBuffer->mCurLogName != log_name
	{
		_mSecBuffer = _mCurBuffer->mNext;
		LogBuffer* temp_curBuf = _mCurBuffer;
		bool isfind = false;
		while (_mSecBuffer != _mCurBuffer)
		{
			if (strcmp(_mSecBuffer->mCurLogName, log_name) == 0 && _mSecBuffer->AvailableLen() > PER_LOG_LEN_LIMIT
				&& _mSecBuffer->mStatus == LogBuffer::FREE)
			{
				isfind = true;
				break;
			}
			else if (_mSecBuffer->Empty())
			{
				memcpy(_mSecBuffer->mCurLogName, log_name, strlen(log_name) + 1);
				isfind = true;
				break;
			}
			else
			{
				_mSecBuffer = _mSecBuffer->mNext;
			}
		}

		if (!isfind)//with no appropriate buffer 
		{
			if (PER_BUFFER_SIZE * (_mBufCnt + 1) > MEM_USE_LIMIT) //
			{
				std::cerr << "Error : no more log space can use\n";
				_mCurBuffer = _mCurBuffer->mNext;
				_mLastErrorTime = _mSys.wSecond;
			}
			else
			{
				LogBuffer* pnext_buf = _mCurBuffer->mNext;
				LogBuffer* pnew_buf = new LogBuffer(PER_BUFFER_SIZE);
				pnew_buf->mPrev = _mCurBuffer;
				_mCurBuffer->mNext = pnew_buf;
				pnew_buf->mNext = pnext_buf;
				pnext_buf->mPrev = pnew_buf;
				_mSecBuffer = pnew_buf;
				_mBufCnt += 1;
				std::cerr << _mBufCnt << std::endl;
				memcpy(_mSecBuffer->mCurLogName, log_name, strlen(log_name) + 1);
				
			}
		}
		if (!_mLastErrorTime)
		{
			_mSecBuffer->AppendLog(log_line, logStr_len);
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
				tell_back = true;
				LogBuffer* pnext_buf = _mCurBuffer->mNext;

				if (pnext_buf->mStatus == LogBuffer::FULL) 
				{
					if (PER_BUFFER_SIZE * (_mBufCnt + 1) > MEM_USE_LIMIT)
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

				bool isfind = false;
				bool different_target = false;
				// Check whether the current log name is consistent with the target write file name
				if (strcmp(log_name, _mCurBuffer->mCurLogName) != 0) 
				{
					different_target = true;
					_mSecBuffer = _mCurBuffer->mNext;
					while (_mSecBuffer != _mCurBuffer)//find a appropriate buffer
					{
						if (strcmp(log_name, _mSecBuffer->mCurLogName) == 0 && _mSecBuffer->AvailableLen() > logStr_len &&
							_mSecBuffer->mStatus == LogBuffer::FREE)
						{

							isfind = true;
							break;
						}
						else if (_mSecBuffer->Empty())
						{
							memcpy(_mSecBuffer->mCurLogName, log_name, strlen(log_name) + 1);
							isfind = true;
							break;
						}
						else
						{
							_mSecBuffer = _mSecBuffer->mNext;
						}
					}
					if (!isfind)//with no appropriate buffer
					{
						if (PER_BUFFER_SIZE * (_mBufCnt + 1) > MEM_USE_LIMIT) 
						{
							std::cerr << "Error : no more log space can use\n";
							_mCurBuffer = pnext_buf;
							_mLastErrorTime = _mSys.wSecond;
						}
						else
						{
							LogBuffer* pnext_buf = _mCurBuffer->mNext;
							LogBuffer* pnew_buf = new LogBuffer(PER_BUFFER_SIZE);
							pnew_buf->mPrev = _mCurBuffer;
							_mCurBuffer->mNext = pnew_buf;
							pnew_buf->mNext = pnext_buf;
							pnext_buf->mPrev = pnew_buf;
							_mSecBuffer = pnew_buf;
							_mBufCnt += 1;
							std::cerr << _mBufCnt << std::endl;
							memcpy(_mSecBuffer->mCurLogName, log_name, strlen(log_name) + 1);
						}
					}
				}
				
				if (!_mLastErrorTime)
				{
					if(!different_target)
						_mCurBuffer->AppendLog(log_line, logStr_len);
					else
						_mSecBuffer->AppendLog(log_line, logStr_len);
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
		ReleaseSemaphore(_hWriteFileSemaphore, 1, NULL);//consumer thread semaphore,persist buffer immediately
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
//                                 消费线程相关函数										  //	
////////////////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI MCLog::CachePersistThreadFunc(LPVOID lpParam)
{
#ifdef  _DEBUG
	std::cerr << "Start Write Log File.\n";
#endif //  DEBUG
	MCLog::LogInstance()->PersistBuffer();
	return NULL;
}

void MCLog::PersistBuffer()
{
	while (true)
	{
		DWORD dwRet = 0;
		
		if (_mPrstBuffer->mStatus == LogBuffer::FREE)
		{
			dwRet = WaitForSingleObject(_hWriteFileSemaphore, BUFF_WAIT_TIME);
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
			if (access(tmpDirPath, 0) != 0) //check whether the log file exists
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