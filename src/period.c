#include <pthread.h>
#include <stdio.h>
#include "types.h"
#include "stream.h"
#include <unistd.h>
#include "sys/socket.h"
#include "connection.h"
#include "bigint.h"

static uint8 buffer[352800];
static uint8 cyclic_end = FALSE;
static pthread_t thread1, thread2;
#define SINGLE_BUFFER_SIZE 88200

static bigint_t syncTime;
static uint16 waitTime;

void *cyclicTask( void *ptr )
{
	 char buf[16];
	 uint32 time;

	 stream_t * stream_p = (stream_t*) ptr;

	 int sock = connection_connect();

	 connection_getTime(&syncTime);

	 bigint_divUint32(&syncTime, 10u);
	 bigint_multUint32(&syncTime, 10u);
	 Stream_SetPosition(stream_p, &syncTime);

	 time = bigint_toUint32(&syncTime);

	 while (TRUE != cyclic_end) {

		 	 	 bigint_t newTime;
		         connection_getTime(&newTime);
                 
                 if (bigint_greatherThan(&newTime, &syncTime, FALSE)) {
                     waitTime--;
                 } else if (bigint_greatherThan(&syncTime, &newTime, FALSE)) {
                     waitTime++;
                 }
                 
                 //printf("diff: %d\n", newTime-syncTime);
                 
                 syncTime = newTime;

		 if (Stream_Length(stream_p) < 5000000) {

			 int toRead = SINGLE_BUFFER_SIZE;
			 sprintf(buf, "MUSI%010u\n", time);
			 send(sock, buf, 15, 0);
			 while (toRead > 0) {
				 toRead -= read(sock, buffer + (SINGLE_BUFFER_SIZE - toRead), toRead);
			 }
			 Stream_Insert(stream_p, buffer, SINGLE_BUFFER_SIZE);
			 time += (SINGLE_BUFFER_SIZE*10/1764);
		 }

		 usleep(1000);
	 }

	 connection_close();

     return 0;
}

void *incTime( void *ptr ) {
        waitTime = 1000u;
    
	while (TRUE != cyclic_end) {
		bigint_addUint32(&syncTime, 1);
		usleep(waitTime);
	}

	return 0;
}

void startCyclic(stream_t* stream)
{
     pthread_create( &thread1, NULL, cyclicTask, (void*) stream);
     pthread_create( &thread2, NULL, incTime, NULL);
}

void closeCyclic() {
	cyclic_end = TRUE;
	pthread_join( thread1, NULL);
	printf("Stream closed gracefully.");
}

void getPlayingTime(bigint_t* time) {
	bigint_clone(time, &syncTime);
}


