#pragma once

class CMyAutoLock;
class CMyKeyLock
{
public:
	CMyKeyLock()
	{
		InitializeCriticalSection(&m_cSession);
	};
	virtual ~CMyKeyLock()
	{
		DeleteCriticalSection(&m_cSession);
	}
protected:
	CRITICAL_SECTION m_cSession;
	friend CMyAutoLock;
};

class CMyAutoLock
{
public:
	CMyAutoLock(CMyKeyLock* pKey)
		: m_pKey(pKey)
	{
		EnterCriticalSection(&(m_pKey->m_cSession));
	};

	virtual ~CMyAutoLock()
	{
		LeaveCriticalSection(&(m_pKey->m_cSession));
	}
protected:
	CMyKeyLock* m_pKey;
};
