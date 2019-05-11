#pragma once

#define _WINSOCKAPI_
#include <windows.h>

////////////////////////////////////////////////////////
// 临界区锁
class CSLock
{ 
public:

	CSLock();
	~CSLock();

	void Lock();
	void UnLock();

private:
	CRITICAL_SECTION m_cs;
};

////////////////////////////////////////////////////////
// 自动锁

class CSAutoLock
{
public:

	CSAutoLock(CSLock& cslock);
	~CSAutoLock();

	void Lock();
	void UnLock();

private:
	CSLock & m_lock;
};