#pragma once

#include "cs_auto_lock_class.h"

CSAutoLock::CSAutoLock(CSLock& cslock):m_lock(cslock)
{
	m_lock.Lock();
}

CSAutoLock::~CSAutoLock()
{
	m_lock.UnLock();
}

void CSAutoLock::Lock()
{
	m_lock.Lock();
}

void CSAutoLock::UnLock()
{
	m_lock.UnLock();
}