#pragma once
//#pragma warning(disable:4996) //close SDL check

#include "mclog_global.h"

//日志默认路径：根目录\Log\当天日期\日志名，如：".\\Log\\2019-12-12(GetSystemDate()获取)\\LogName.txt",日志名的".txt"可缺省
//自定义日志路径格式: ".\\路径\\" , ".\\路径" , "./路径" , "./路径/"
#define LOG_DEFAULT_PATH ".\\Log\\"
//日志文件最大大小限制    1Gb
#define MEM_USE_LIMIT (1u * 1024 * 1024 * 1024)
//单条日志长度限制        4Kb
#define LOG_LEN_LIMIT (4 * 1024)
//消费者线程等待信号时间   单位ms
#define BUFF_WAIT_TIME (500)
//单缓存区长度对应宏
#define PER_BUFFER_SIZE (_mPerBufSize)

class MCLOG_API MCLog
{
public:
    static MCLog* LogInstance()
    {
        Init();
        return _mInstance;
    }
private:
    static void Init()
    {
        if (!_mInstance)
        {
            _mInstance = new MCLog();
            memcpy(_mInstance->_mLogPath, LOG_DEFAULT_PATH, strlen(LOG_DEFAULT_PATH) + 1);
            _mInstance->_hWriteFileSemaphore = CreateSemaphore(NULL, 0, 1, NULL);		//初始signal状态:  unsignnal;
            ::InitializeCriticalSection(&(_mInstance->_hCS_CurBufferLock));
            ::InitializeCriticalSection(&(_mInstance->_hCS_TempBufferLock));
            HANDLE LogConsumerThread = CreateThread(NULL, NULL, CachePersistThreadFunc, NULL, 0, NULL);
            if (LogConsumerThread != 0)
                CloseHandle(LogConsumerThread);
        }
    }

public:
    //设置日志路径,不调用该函数则使用默认路径
    void SetLogPath(const char* log_path = LOG_DEFAULT_PATH);
    //将日志写入缓存
    void WriteLogCache(const char* log_name, const char* log_str);

private:
    LogBuffer* GetRelevantBuffer(const char* log_name); //寻找一个空的缓存区

private:
    MCLog();
    ~MCLog();

private:
    static MCLog*    _mInstance;           //单例
    static uint32_t  _mPerBufSize;         //单缓存区长度大小 1024 * 1024 == 1Mb

    HANDLE           _hWriteFileSemaphore; //某一缓存区满该信号量唤醒消费者线程进行文件写入
    CRITICAL_SECTION _hCS_CurBufferLock;   //临界区 同步_mCurBuffer
    CRITICAL_SECTION _hCS_TempBufferLock;   //临界区 同步_mCurBuffer
    CRITICAL_SECTION _m_CSLock;
    SYSTEMTIME       _mSys;
    uint32_t         _mPid;
    FILE* _mFp;
    char* _mLogPath;            //日志文件路径
    char* _mSysDate;            //系统日期,例:2019-12-12
    char* _mLogFileLocation;    //当且消费者线程正在写入的日志完整路径,如：".\\Log\\2019-12-12(调用FlushLogPath())\\LogName.txt"
    int              _mBufCnt;             //缓存区块数量
    LogBuffer*       _mCurBuffer;          //当前正在写入数据的缓存区指针
    LogBuffer*       _mPrstBuffer;         //当前正在将缓存数据转录进文件的缓存区指针
    uint64_t         _mLastErrorTime;          //日志发生错误的时间,日志正常运行值为0

    bool CreateFilePath(const char* log_path);
    bool OpenFile(const char* log_name);

private:  //消费线程相关函数
    //消费线程函数，从缓存拿数据写入文本文件
    static DWORD WINAPI CachePersistThreadFunc(LPVOID lpParam);
    //缓存写入文本文件
    void BufferPersist();
    //获取系统日期,例:2019-12-12
    void GetSystemDate(char* log_date);
};

#define LOG_INIT()\
do{\
    MCLog::LogInstance();\
}while (0)

#define SET_LOGPATH(log_path)\
do{\
    MCLog::LogInstance()->SetLogPath(log_path); \
}while (0)

#define WRITE_LOG(log_name,log_str)\
do{\
    MCLog::LogInstance()->WriteLogCache(log_name, log_str);\
}while (0)




