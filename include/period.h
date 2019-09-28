
#ifndef PERIOD_H
#define PERIOD_H

#include "stream.h"

extern void *cyclicTask( void *ptr );
extern void *incTime( void *ptr );
extern void startCyclic(stream_t* stream);
extern void closeCyclic();
extern void getPlayingTime(bigint_t* time);

#endif
