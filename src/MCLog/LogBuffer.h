#pragma once
#include <cstdio>
#include <cstdint>
#include <ctime>

//日志缓存区节点-循环链表节点
class LogBuffer
{
public:
    enum BufferStatus
    {
        FREE,
        FULL
    };

    LogBuffer(uint32_t len) :
        mStatus(FREE),
        mPrev(NULL),
        mNext(NULL),
        _mTotalLen(len),
        _mUsedLen(0)
    {
        _mTotalLen = len;
        mCurLogName = new char[MAX_PATH];
        memset(mCurLogName, 0, MAX_PATH);
        _mCacheData = new char[len];
        memset(_mCacheData, 0, len);
        if (!_mCacheData)
        {
            std::cerr << "Error : no space to allocate _mCacheData\n";
            exit(1);
        }
    }

    void Clear()
    {
        memset(_mCacheData, 0, _mTotalLen);
        memset(mCurLogName, 0, MAX_PATH);
        _mUsedLen = 0;
        mStatus = FREE;
    }

    uint32_t AvailableLen() const
    {
        return this->_mTotalLen - this->_mUsedLen;
    }

    bool Empty() const
    {
        return _mUsedLen == 0 && mCurLogName[0] == '\0';
    }

    void AppendLog(const char* log_line, uint32_t len)
    {
        if (AvailableLen() < len)
            return;
        memcpy(_mCacheData + _mUsedLen, log_line, len);
        _mUsedLen += len;
    }

    void WriteFile(FILE* fp)
    {
        uint32_t writeLen = fwrite(_mCacheData, 1, _mUsedLen, fp);
        if (writeLen != _mUsedLen) //写入长度与缓存区文本长度不一致则判断写入文本失败
        {
            std::cerr << "Error : write log to disk error,writeLen: "<< writeLen<<std::endl;
        }
    }

public:
    BufferStatus    mStatus;   //缓存区状态
    LogBuffer* mPrev;   //缓存区块前一个缓存区指针
    LogBuffer* mNext;   //缓存区块下一个缓存区指针
    char* mCurLogName;    //缓存区块对应日志文件名称

private:
    uint32_t _mTotalLen;  //单区块空间大小
    uint32_t _mUsedLen;   //区块已用空间
    char* _mCacheData; //区块数据 

};
