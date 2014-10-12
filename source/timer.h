/**********************************************************************************
**  本模块为学习 skynet 时将其内部的 定时器模块提取出来的Cpp实现方式
**  
***********************************************************************************/

#ifndef __2014_09_24_TIMER_H__
#define __2014_09_24_TIMER_H__

#include <cstdint>
#include <time.h>

class Timer
{
public:
	static Timer* getInstance();
	// 初始化定时器
	int32_t	initTimer();

	// 定时器内部更新，此接口应该由一个放在一个线程中不断的去更新执行
	int32_t	updateTimer();

	//向定时器添加定时操作
	// delay_msec 单位为10 毫秒
	int32_t	addTimeout(uint32_t delay_msec, void* data, uint32_t data_size);

	//获取当前时间戳
	uint64_t getTime();

	//获取开始运行时间戳
	uint64_t getStartTime();

	//获取当前系统的本地时间
	int32_t getTimeOfDay(struct timeval *tp, void *tzp);
	//获取当前系统的本地时间
	struct tm getSystemLocalTime();
protected:
	Timer();
	~Timer();
};
#endif//__2014_09_24_TIMER_H__