#ifndef _CLOCK_H_
#define _CLOCK_H_

extern const int CLOCK_MONOTONIC;

int clock_gettime(int X, struct timespec *tv);

#endif