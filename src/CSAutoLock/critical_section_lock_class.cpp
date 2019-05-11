#pragma once

#include "cs_auto_lock_class.h"

CSLock::CSLock()
{
	::InitializeCriticalSection(&m_cs);
}
CSLock::~CSLock()
{
	::InitializeCriticalSection(&m_cs);
}

void CSLock::Lock()
{
	::EnterCriticalSection(&m_cs);
}

void CSLock::UnLock()
{
	::LeaveCriticalSection(&m_cs);
}