#if !defined(AFX_COMMONAUTOLOCK_H__9FC30C9A_9774_41B1_BB33_5A737FC48538__INCLUDED_)
#define AFX_COMMONAUTOLOCK_H__9FC30C9A_9774_41B1_BB33_5A737FC48538__INCLUDED_

class CCommonAutoLock;

class  CCommonKeyLock
{
	CRITICAL_SECTION cSection;
	friend CCommonAutoLock;
public:
	CCommonKeyLock();
	virtual ~CCommonKeyLock();
};

class  CCommonAutoLock
{
	CCommonKeyLock* m_pKey;
public:
	CCommonAutoLock(CCommonKeyLock* pKey);
	virtual ~CCommonAutoLock(void);
};


#endif // !defined(AFX_COMMONAUTOLOCK_H__9FC30C9A_9774_41B1_BB33_5A737FC48538__INCLUDED_)