#ifndef __AUTOLOCK_H_
#define __AUTOLOCK_H_

class CAutoLock
{
public:
	CAutoLock(pthread_mutex_t *pMutex)
		:m_pMutex(pMutex)
	{
		if(m_pMutex!=NULL)
		{
			pthread_mutex_lock(m_pMutex);
		}
	}
	~CAutoLock()
	{
		if(m_pMutex!=NULL)
		{
			pthread_mutex_unlock(m_pMutex);
			m_pMutex = NULL;
		}
	}

private:
	pthread_mutex_t *m_pMutex;
};

#endif
