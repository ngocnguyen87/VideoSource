#include "StdAfx.h"
#include ".\commonautolock.h"

CCommonKeyLock::CCommonKeyLock()
{
	InitializeCriticalSection(&cSection);
}

CCommonKeyLock::~CCommonKeyLock()
{
	DeleteCriticalSection(&cSection);
}

CCommonAutoLock::CCommonAutoLock(CCommonKeyLock* pKey) : m_pKey(pKey)
{
	EnterCriticalSection(&m_pKey->cSection);
}

CCommonAutoLock::~CCommonAutoLock(void)
{
	LeaveCriticalSection(&m_pKey->cSection);
}
