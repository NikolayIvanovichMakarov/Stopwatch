#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdint.h>
#include <ncurses.h>

typedef enum stopwatch_e
{
	sw_waiting,
	sw_running,
	sw_paused
} stopwatch_t;


uint8_t g_b_tmr_run = 1;
uint8_t g_b_paused = 0;
uint8_t g_b_runned = 0;
uint8_t g_b_cleared = 0;

void * thrd_timer_main_cycle()
{
	long int sec,usec,min,hour;
	struct timeval start;
	struct timeval stop;
	struct timeval accumulated_time = {0, 0};
	stopwatch_t current_state;

	#define CLEAR_TIMER()\
		accumulated_time.tv_sec = 0;\
		accumulated_time.tv_usec = 0;\
		hour = 0;\
		min = 0;\
		sec = 0;\
		usec = 0;

	#define START_STOPWATCH()\
		gettimeofday(&start, NULL);

	#define PRINT_TIME()\
		clear();\
		printw("hour: %ld min: %ld sec: %ld usec: %ld\n",hour, min, sec, usec);\
		refresh();

	while(g_b_tmr_run)
	{
		switch(current_state)
		{
			case sw_waiting:
				if (g_b_paused == 1)
				{
					g_b_paused = 0;
				}
				else if (g_b_cleared == 1)
				{
					g_b_cleared = 0;
				}
				else if (g_b_runned == 1)
				{
					g_b_runned = 0;
					current_state = sw_running;
					START_STOPWATCH();
				}
			break;
			case sw_running:
				gettimeofday(&stop, NULL);

				sec  = ((stop.tv_sec - start.tv_sec) + accumulated_time.tv_sec);
				usec = ((stop.tv_usec - start.tv_usec) + accumulated_time.tv_usec);
				
				if (usec > 1000000)
				{
					sec += usec / 1000000;
					usec = usec % 1000000;
				}
				else if (usec < 0)
				{
					sec = sec-1;
					usec = usec + 1000000;
				}

				if (g_b_runned == 1)
				{
					g_b_runned = 0;
				}
				else if (g_b_paused == 1)
				{
					g_b_paused = 0;
					accumulated_time.tv_sec = sec;
					accumulated_time.tv_usec = usec;
					current_state = sw_paused;
				}
				else if (g_b_cleared == 1)
				{
					g_b_cleared = 0;
					//current_state = sw_waiting;

					//CLEAR_TIMER();
					START_STOPWATCH();
				}
				else
				{
					min = sec / 60;
					hour = min / 60;
					hour = hour % 24;
					min = min % 60;
					sec = sec % 60;
					
					PRINT_TIME();
					usleep(10000);
				}
			break;
			case sw_paused:
				if (g_b_paused == 1)
				{
					g_b_paused = 0;
				}
				else if (g_b_cleared == 1)
				{
					g_b_cleared = 0;

					CLEAR_TIMER();
					PRINT_TIME();
				} 
				else if (g_b_runned == 1)
				{
					g_b_runned = 0;
					current_state = sw_running;

					START_STOPWATCH();
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
	pthread_create(&thrd_timer, &attr, thrd_timer_main_cycle, NULL);
	uint8_t main_program = 1;
	char ch;

	initscr();
	keypad(stdscr, true); 
	curs_set(0);

	while(main_program == 1)
	{
		ch = getch();
		//scanf("%c",&ch);
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
			case 'c':
				g_b_cleared = 1;
			break;
		}
	}

	endwin();
	return 0;
}
