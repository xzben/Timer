/********************************************************************************
*	�ļ�����:	mutex.h															*
*	����ʱ�䣺	2014/06/05														*
*	��   �� :	xzben															*
*	�ļ�����:	ϵͳ��ʹ�õ��̰߳�ȫ��											*
*********************************************************************************/


#include <mutex>

class std::timed_mutex;

/*
*	��������ʹ�� C++11 �ṩ�Ķ�ʱ������ʵ��
*	����ʹ�õ��Ƕ�ʱ��
*/
class Mutex
{
public:
	Mutex();
	virtual ~Mutex();
	void lock();
	bool try_lock(int milliseconds = 200);
	void unlock();
protected:
	std::timed_mutex	m_lock;
};

class Guard
{
public:
	Guard(Mutex* pMutex);
	~Guard();
private:
	Mutex	*m_pGuardMuext;
};