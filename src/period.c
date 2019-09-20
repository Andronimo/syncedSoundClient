#include <pthread.h>
#include <stdio.h>
#include "types.h"
#include "stream.h"
#include <unistd.h>

extern int connection_connect(void);
extern int connection_close(void);
extern uint32 connection_getTime(void);

static uint8 buffer[352800];
static uint8 cyclic_end = FALSE;
static pthread_t thread1, thread2;
#define SINGLE_BUFFER_SIZE 88200

static uint32 playingTime;

void *cyclicTask( void *ptr )
{
	 char buf[16];

	 stream_t * stream_p = (stream_t*) ptr;

	 int sock = connection_connect();

	 uint32 time = connection_getTime() / 10 * 10;
	 playingTime = time;

	 while (TRUE != cyclic_end) {

		 uint32 currentTime = connection_getTime();

		 playingTime = currentTime;

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
	while (TRUE != cyclic_end) {
		playingTime += 1;
		usleep(935);
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

uint32 getPlayingTime() {
	return playingTime;
}


