#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdint.h>
typedef enum stopwatch_e
{
	sw_waiting,
	sw_running,
	sw_paused
} stopwatch_t;

stopwatch_t g_current_state;

time_t g_current_time;
struct tm* g_p_time_info;

struct timeval g_start;
struct timeval g_stop;
struct timeval g_accumulated_time = {0, 0};

void start()
{
	gettimeofday(&g_start, NULL);
}

void print_current_time()
{
	gettimeofday(&g_stop, NULL);
	printf("sec: %lu usec: %lu \n", (g_stop.tv_sec - g_start.tv_sec), (g_stop.tv_usec - g_start.tv_usec)); 
}

uint8_t g_b_tmr_run = 1;
uint8_t g_b_paused = 0;
uint8_t g_b_runned = 0;

void * thrd_timer_main_cycle()
{
	long int sec,usec,min,hour;
	while(g_b_tmr_run)
	{
		switch(g_current_state)
		{
			case sw_waiting:
				if (g_b_runned == 1)
				{
					g_b_runned = 0;
					if (g_b_paused == 1)
						g_b_paused = 0;
					g_current_state = sw_running;
					start();
				}
			break;
			case sw_running:
				gettimeofday(&g_stop, NULL);

				sec  = ((g_stop.tv_sec - g_start.tv_sec) + g_accumulated_time.tv_sec);
				usec = ((g_stop.tv_usec - g_start.tv_usec) + g_accumulated_time.tv_usec);
				
				if (usec > 1000000)
				{
					sec += usec / 1000000;
					usec = usec % 1000000;
				}
				if (usec < 0)
				{
					sec = sec-1;
					usec = usec + 1000000;
				}
				if (g_b_paused == 1)
				{
					g_b_paused = 0;
					g_accumulated_time.tv_sec = sec;
					g_accumulated_time.tv_usec = usec;

					g_current_state = sw_paused;
				}

				min = sec / 60;
				hour = min / 60;
				hour = hour % 24;
				min = min % 60;
				sec = sec % 60;

				printf("hour: %ld min: %ld sec: %ld usec: %ld \n",hour, min, sec, usec);
				
			break;
			case sw_paused:
				if (g_b_paused == 1)
					g_b_paused = 0;
				if (g_b_runned == 1)
				{
					g_b_runned = 0;
					g_current_state = sw_running;
					start();
				}
			break;
			default:
			break;
		}
	}
	pthread_exit(0);
}
int main()
{
	pthread_t thrd_timer;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	char str[20];
	char ch;
	pthread_create(&thrd_timer, &attr, thrd_timer_main_cycle, NULL);
	uint8_t main_program = 1;
	while(main_program == 1)
	{
		scanf("%c",&ch);
		switch(ch)
		{
			case 'q':
				g_b_tmr_run = 0;
				pthread_join(thrd_timer, NULL);
				main_program = 0;

			break;
			case 'r':
				g_b_runned = 1;
			break;
			case 'p':
				g_b_paused = 1;
			break;
		}
	}



	return 0;
}