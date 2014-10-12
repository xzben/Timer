#include <iostream>
#include <thread>
#include "timer.h"
using namespace std;


void _timer()
{
	while(1)
	{
		Timer::getInstance()->updateTimer();
	}
}

int main()
{
	printf("\007The current date/time is: %s", asctime(&Timer::getInstance()->getSystemLocalTime()));
	
	Timer::getInstance()->initTimer();
	std::thread  thd(_timer);

	printf("start time: %lld\n", Timer::getInstance()->getStartTime());
	char* str[] = { 
		"this is timeout one", 
		"this is timeout two",
		"this is timeout three",
		"this is timeout four",
		"this is timeout five",
		nullptr,
	};
	
	for(int i = 0 ; str[i] != nullptr; i++)
	{
		if( 0 !=  Timer::getInstance()->addTimeout((i+1)*100, str[i], strlen(str[i]) + 1) )
		{
			printf("error....\n");
		}
		else
		{
			printf("add timeout success %d\n", i);
		}
	}
	
	thd.join();
	return 0;
}