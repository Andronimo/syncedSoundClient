
#ifndef CONNECTION_H
#define CONNECTION_H

#include "stream.h"

extern int connection_connect();
extern void connection_getTime(bigint_t* bigint);
extern void connection_close();
extern void connection_setServer(char* url, int p);

#endif
