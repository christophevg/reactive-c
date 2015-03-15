#include "time.h"

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

// via: http://stackoverflow.com/questions/5167269/

struct timespec now() {
  struct timespec ts;
#ifdef __MACH__
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  ts.tv_sec = mts.tv_sec;
  ts.tv_nsec = mts.tv_nsec;
#else
  
#endif
  return ts;
}

double diff(struct timespec start, struct timespec end) {
	struct timespec temp;
	if((end.tv_nsec - start.tv_nsec) < 0) {
		temp.tv_sec  = end.tv_sec - start.tv_sec - 1;
		temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
	} else {
		temp.tv_sec  = end.tv_sec - start.tv_sec;
		temp.tv_nsec = end.tv_nsec - start.tv_nsec;
	}
	return temp.tv_sec + temp.tv_nsec / 1000000000.0;
}
