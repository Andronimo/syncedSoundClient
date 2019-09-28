
#ifndef CONNECTION_H
#define CONNECTION_H

#include "stream.h"

extern int connection_connect();
extern uint32 connection_getTime(uint32 oldTime);
extern void connection_close();
extern void connection_setServer(char* url, int p);

#endif
