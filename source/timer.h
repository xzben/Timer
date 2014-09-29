/**********************************************************************************
**  本模块为学习 skynet 时将其内部的 定时器模块提取出来的Cpp实现方式
**  
***********************************************************************************/

#ifndef __2014_09_24_TIMER_H__
#define __2014_09_24_TIMER_H__

#include <cstdint>

class Timer
{
public:
	static Timer* get_instance();
	// 初始化定时器
	int32_t	init_timer();

	// 定时器内部更新，此接口应该由一个放在一个线程中不断的去更新执行
	int32_t	update_timer();

	//向定时器添加定时操作
	// delay_msec 单位为10 毫秒
	int32_t	add_timeout(uint32_t delay_msec, void* data, uint32_t data_size);

	//获取当前时间戳
	uint64_t get_time();

	//获取开始运行时间戳
	uint64_t get_start_time();
protected:
	Timer();
	~Timer();
};
#endif//__2014_09_24_TIMER_H__