#include"mutex.h"


Mutex::Mutex()
{

}

Mutex::~Mutex()
{
	
}

void Mutex::lock()
{
	m_lock.lock();
}

bool Mutex::try_lock(int milliseconds /*= 200*/)
{
	return m_lock.try_lock_for(std::chrono::milliseconds(milliseconds));
}

void Mutex::unlock()
{
	m_lock.unlock();
}


Guard::Guard(Mutex* pMutex)
{
	m_pGuardMuext = pMutex;
	if (nullptr != m_pGuardMuext)
	{
		m_pGuardMuext->lock();
	}
}

Guard::~Guard()
{
	if (nullptr != m_pGuardMuext)
	{
		m_pGuardMuext->unlock();
	}
}