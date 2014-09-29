/**********************************************************************************
**  ��ģ��Ϊѧϰ skynet ʱ�����ڲ��� ��ʱ��ģ����ȡ������Cppʵ�ַ�ʽ
**  
***********************************************************************************/

#ifndef __2014_09_24_TIMER_H__
#define __2014_09_24_TIMER_H__

#include <cstdint>

class Timer
{
public:
	static Timer* get_instance();
	// ��ʼ����ʱ��
	int32_t	init_timer();

	// ��ʱ���ڲ����£��˽ӿ�Ӧ����һ������һ���߳��в��ϵ�ȥ����ִ��
	int32_t	update_timer();

	//��ʱ����Ӷ�ʱ����
	// delay_msec ��λΪ10 ����
	int32_t	add_timeout(uint32_t delay_msec, void* data, uint32_t data_size);

	//��ȡ��ǰʱ���
	uint64_t get_time();

	//��ȡ��ʼ����ʱ���
	uint64_t get_start_time();
protected:
	Timer();
	~Timer();
};
#endif//__2014_09_24_TIMER_H__