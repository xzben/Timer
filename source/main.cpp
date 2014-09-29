#include <iostream>
#include <thread>
#include "timer.h"
using namespace std;


void _timer()
{
	while(1)
	{
		Timer::get_instance()->update_timer();
	}
}

int main()
{

	Timer::get_instance()->init_timer();
	std::thread  thd(_timer);

	printf("start time: %lld\n", Timer::get_instance()->get_start_time());
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
		if( 0 !=  Timer::get_instance()->add_timeout((i+1)*100, str[i], strlen(str[i]) + 1) )
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