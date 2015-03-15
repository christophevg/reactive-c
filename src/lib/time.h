#ifndef __LIB_TIME_H
#define __LIB_TIME_H

#include <time.h>
#include <sys/time.h>

struct timespec now();
double diff(struct timespec, struct timespec);

#endif
