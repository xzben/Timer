#include "timer.h"
#include "mutex.h"
#include <cstdio>
#include <iostream>

#ifdef _WIN32
	#include <windows.h>
#else
	#include <sys/time.h>
#endif

//////////////////////////////////////////////////////////////////////////
// 华丽分割线 // 以下为内部配置参数, 此配置
#define TIMER_NEAR_SHIFT				8
#define TIMER_LEVEL_SHIFT				6
#define TIMER_NEAR						(1<<TIMER_NEAR_SHIFT)
#define TIMER_LEVEL						(1<<TIMER_LEVEL_SHIFT)
#define TIMER_NEAR_MASK					(TIMER_NEAR-1)
#define TIMER_LEVEL_MASK				(TIMER_LEVEL-1)


//////////////////////////////////////////////////////////////////////////
// 华丽分割线 // 一下为内部实现的细节部分
#ifdef _WIN32
int gettimeofday(struct timeval *tp, void *tzp)
{
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;
	GetLocalTime(&wtm);
	tm.tm_year     = wtm.wYear - 1900;
	tm.tm_mon     = wtm.wMonth - 1;
	tm.tm_mday     = wtm.wDay;
	tm.tm_hour     = wtm.wHour;
	tm.tm_min     = wtm.wMinute;
	tm.tm_sec     = wtm.wSecond;
	tm. tm_isdst    = -1;
	clock = mktime(&tm);
	tp->tv_sec = clock;
	tp->tv_usec = wtm.wMilliseconds * 1000;

	return (0);
}
#endif

struct TimerNode
{
	struct TimerNode *next;
	uint64_t expire_time;

	//节点附带的数据
	uint32_t	data_len;
	void		*pData;
};


struct TimerList
{
	TimerNode	head;
	TimerNode*	tail;

	TimerList()
	{
		head.next = nullptr;
		tail = &head;
	}

	void link_node(TimerNode* node)
	{
		this->tail->next = node;
		this->tail = node;
		node->next = nullptr;
	}

	TimerNode* link_clear()
	{
		TimerNode* ret = this->head.next;

		this->head.next = nullptr;
		this->tail = &this->head;

		return ret;
	}
};

/*
*	时间单位为 10 毫秒
*/
class TimerCore
{
public:
	TimerCore();
	~TimerCore();

	static TimerCore*	get_instance();
	uint64_t			get_systime(); //返回当前系统时间(10毫秒单位)
	uint64_t			get_time();
	uint32_t			get_time_msec();
	uint32_t			get_time_sec();
	uint64_t			get_start_time();

	int32_t				init();	
	void				dispatch_list(struct TimerNode *current);
	void				timer_execute();
	void				timer_shift();
	void				update_time(uint64_t cp);
	int32_t				update();
	int32_t				add_node(TimerNode* node);
	int32_t				add_timeout(uint32_t delay_msec, void* data = nullptr, uint32_t data_size = 0);
private:
	TimerList		m_near[TIMER_NEAR];
	TimerList		m_wait[4][TIMER_LEVEL];
	Mutex			m_lock;					// 内核锁
	uint32_t		m_time;					// 从服务器开始经历的时间点
	uint32_t		m_unCurrentSec;			// 当前时间的秒数部分
	uint32_t		m_unCurrentMSec;		// 当前时间的毫秒部分
	uint64_t		m_unCurTimePoint;		// 当前时间戳	
	uint64_t		m_unStartTimePoint;		// 服务器开始时的时间戳
};


TimerCore* TimerCore::get_instance()
{
	static TimerCore s_instance;

	return &s_instance;
}

TimerCore::TimerCore()
{
	m_time = 0;
	m_unCurrentSec		= 0;
	m_unCurrentMSec		= 0;
	m_unCurTimePoint	= 0;
	m_unStartTimePoint	= 0;
}

TimerCore::~TimerCore()
{

}

//返回当前系统时间(10毫秒单位)
uint64_t	TimerCore::get_systime()
{
	uint64_t t;

	struct timeval tv;
	gettimeofday(&tv, NULL);
	t = (uint64_t)tv.tv_sec * 100;
	t += tv.tv_usec / 10000;

	return t;
}

uint32_t	TimerCore::get_time_msec()
{
	return m_unCurrentMSec;
}
uint32_t	TimerCore::get_time_sec()
{
	return m_unCurrentSec;
}

uint64_t	TimerCore::get_time()
{
	return m_unCurTimePoint;
}

uint64_t	TimerCore::get_start_time()
{
	return m_unStartTimePoint;
}

int32_t TimerCore::init()
{
	uint64_t cp = get_systime();

	m_time = 0;
	m_unCurrentSec = cp/100;
	m_unCurrentMSec = 10*(cp - m_unCurrentSec);
	m_unCurTimePoint = cp;
	m_unStartTimePoint = cp;

	return 0;
}

void TimerCore::dispatch_list(struct TimerNode *current) 
{
	while(current) 
	{
		uint32_t datalen = current->data_len;
		void*	 data	 = current->pData;
		uint64_t expire = current->expire_time;
		////////////////////////////////////////
		//此处执行要处理的逻辑, 不要再定时器模块中执行具体的逻辑
		//应该将具体的执行发送给工作线程执行，否则会导致定时器产生误差
		printf("%s\n", data);
		std::cout<< data << "-----"<< m_unCurTimePoint << std::endl;
		free(data);
		///////////////////////////////////////
		struct TimerNode *tmp = current;
		current = current->next;
		free(tmp);
	}
}

void	TimerCore::timer_execute()
{
	m_lock.lock();

	int idx = m_time & TIMER_NEAR_MASK;
	while (m_near[idx].head.next) {
		struct TimerNode *current = m_near[idx].link_clear();
		m_lock.unlock();

		// dispatch_list don't need lock T
		dispatch_list(current);

		m_lock.lock();
	}

	m_lock.unlock();
}

void	TimerCore::timer_shift()
{
	m_lock.lock();

	int mask = TIMER_NEAR;
	int time = (++m_time) >> TIMER_NEAR_SHIFT;
	int i=0;

	while ((m_time & (mask-1))==0) {
		int idx=time & TIMER_LEVEL_MASK;
		if (idx!=0) {
			--idx;
			struct TimerNode *current = m_wait[i][idx].link_clear();
			while (current) {
				struct TimerNode *temp=current->next;
				add_node(current);
				current=temp;
			}
			break;				
		}
		mask <<= TIMER_LEVEL_SHIFT;
		time >>= TIMER_LEVEL_SHIFT;
		++i;
	}	
	m_lock.unlock();
}

void	TimerCore::update_time(uint64_t new_cp)
{
	m_lock.lock();

	//std::cout<<"current time point:"<<new_cp<<std::endl;

	uint64_t old_cp = m_unCurTimePoint;
	uint32_t diff = (uint32_t)(new_cp - old_cp);

	uint32_t msec = m_unCurrentMSec;
	uint32_t new_sec  = m_unCurrentSec;
	uint32_t new_msec = msec + diff;

	if (new_msec < msec) 
	{
		// when cs > 0xffffffff(about 497 days), time rewind
		new_sec += 0xffffffff / 100;
	}

	m_unCurTimePoint	= new_cp;
	m_unCurrentSec		= new_sec;
	m_unCurrentMSec		= new_msec;

	m_lock.unlock();
}

int32_t TimerCore::update()
{
	timer_execute();
	
	timer_shift();
		
	timer_execute();
	return 0;
}

int32_t		TimerCore::add_node(TimerNode* node)
{
	if(nullptr == node) return 1; //error
	
	uint64_t expire = node->expire_time;
	uint64_t curtime = m_time;

	//如果节点终止时间在临近执行的时间范围内
	if( (expire|TIMER_NEAR_MASK) == (curtime|TIMER_NEAR_MASK) )
	{
		m_near[expire&TIMER_NEAR_MASK].link_node(node);
	}
	else//如果不在临近执行的LEVEL 在计算其所在level将节点放入其列表中
	{
		uint32_t mask = TIMER_NEAR<<TIMER_NEAR_SHIFT;
		int level = 0;
		for(level = 0; level < 3; level++)
		{
			if( (expire|(mask-1)) == (curtime|(mask-1)) )
				break;
			
			mask = mask << TIMER_NEAR_SHIFT;
		}
		uint32_t idx = ((expire>>(TIMER_NEAR_SHIFT + level*TIMER_LEVEL_SHIFT)) & TIMER_LEVEL_MASK)-1;
		m_wait[level][idx].link_node(node);
	}

	return 0;
}

int32_t		TimerCore::add_timeout(uint32_t delay_msec, void* data, uint32_t data_size)
{
	// 分配一个时间节点，并将其附带参数保存在其中
	TimerNode* newNode = (TimerNode*)malloc( sizeof(TimerNode) );
	if(nullptr == newNode) // error 
	{
		return 1;
	}
	if(nullptr == data) 
	{
		data_size = 0;
	}

	newNode->data_len = data_size;
	newNode->pData = (void*)malloc(data_size);
	if(nullptr == newNode->pData)
	{
		return 1;
	}

	if( data_size > 0 ) 
	{
		memcpy(newNode->pData, data, data_size);
	}

	m_lock.lock();

	uint64_t expire_time = m_time + delay_msec;
	newNode->expire_time = expire_time;
	newNode->next = nullptr;
	add_node(newNode);
	m_lock.unlock();

	return 0;
}
//////////////////////////////////////////////////////////////////////////
// 华丽分割线，一下为对外抛出接口的实现部分

Timer* Timer::getInstance()
{
	static Timer s_instancel;

	return &s_instancel;
}

Timer::Timer()
{

}

Timer::~Timer()
{

}

//获得当前系统时间
int32_t Timer::getTimeOfDay(struct timeval *tp, void *tzp)
{
	return ::gettimeofday(tp, tzp);
}
// 初始化定时器
int32_t	Timer::initTimer()
{
	return TimerCore::get_instance()->init();
}

// 定时器内部更新，此接口应该由一个放在一个线程中不断的去更新执行
int32_t	Timer::updateTimer()
{
	uint64_t systime = TimerCore::get_instance()->get_systime();
	uint64_t cp	 = TimerCore::get_instance()->get_time();
	
	if(systime < cp)// 出错.....
	{
		std::cout<<"error"<<std::endl;
	}
	else if( systime != cp)
	{
		uint32_t diff = systime - cp;
		TimerCore::get_instance()->update_time(systime);

		for (int i=0;i<diff;i++) {
			TimerCore::get_instance()->update();
		}
	}
	
	return 0;
}

//向定时器添加定时操作
int32_t	Timer::addTimeout(uint32_t delay_msec, void* data, uint32_t data_size)
{
	return TimerCore::get_instance()->add_timeout(delay_msec, data, data_size);
}

//获取当前时间戳
uint64_t Timer::getTime()
{
	return TimerCore::get_instance()->get_time();
}

//获取开始运行时间戳
uint64_t Timer::getStartTime()
{
	return TimerCore::get_instance()->get_start_time();
}

struct tm Timer::getSystemLocalTime()
{
	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	
	return *timeinfo;
}
