/********************************************************************************
*	文件名称:	mutex.h															*
*	创建时间：	2014/06/05														*
*	作   者 :	xzben															*
*	文件功能:	系统中使用的线程安全锁											*
*********************************************************************************/


#include <mutex>

class std::timed_mutex;

/*
*	互斥锁，使用 C++11 提供的定时锁对象实现
*	本锁使用的是定时锁
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