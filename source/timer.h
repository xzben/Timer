/**********************************************************************************
**  ��ģ��Ϊѧϰ skynet ʱ�����ڲ��� ��ʱ��ģ����ȡ������Cppʵ�ַ�ʽ
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
	// ��ʼ����ʱ��
	int32_t	initTimer();

	// ��ʱ���ڲ����£��˽ӿ�Ӧ����һ������һ���߳��в��ϵ�ȥ����ִ��
	int32_t	updateTimer();

	//��ʱ����Ӷ�ʱ����
	// delay_msec ��λΪ10 ����
	int32_t	addTimeout(uint32_t delay_msec, void* data, uint32_t data_size);

	//��ȡ��ǰʱ���
	uint64_t getTime();

	//��ȡ��ʼ����ʱ���
	uint64_t getStartTime();

	//��ȡ��ǰϵͳ�ı���ʱ��
	int32_t getTimeOfDay(struct timeval *tp, void *tzp);
	//��ȡ��ǰϵͳ�ı���ʱ��
	struct tm getSystemLocalTime();
protected:
	Timer();
	~Timer();
};
#endif//__2014_09_24_TIMER_H__