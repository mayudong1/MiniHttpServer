#ifndef __AUTOLOCK_H_
#define __AUTOLOCK_H_

class CAutoLock
{
public:
	CAutoLock(CRITICAL_SECTION *pCriticalSection)
		:m_pCriticalSection(pCriticalSection)
	{
		if(m_pCriticalSection!=NULL)
		{
			EnterCriticalSection(m_pCriticalSection);
		}
	}
	~CAutoLock()
	{
		if(m_pCriticalSection!=NULL)
		{
			LeaveCriticalSection(m_pCriticalSection);
			m_pCriticalSection = NULL;
		}
	}

private:
	CRITICAL_SECTION *m_pCriticalSection;
};

#endif
