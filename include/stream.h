

#ifndef STREAM_H
#define STREAM_H

#include "types.h"
#include <pthread.h>

typedef struct stream {
	uint32 next;
	uint32 last;
	uint32 length;
	uint32 position;
	uint8 full;
	uint8* data;
	pthread_mutex_t mutex;
} stream_t;

extern uint32 Stream_Length(stream_t* st);
extern uint32 Stream_GetRest(stream_t* st);
extern uint8 Stream_PutElement(stream_t* st, uint8* data, uint32 length);
extern void Stream_Print(stream_t* st);
extern uint32 Stream_Poll(stream_t* st, uint8* data, uint8 remove);
extern void Stream_Init(stream_t* st, uint32 length, uint32 startPosition);
extern void Stream_Close(stream_t* st);
extern uint32 Stream_LengthTop(stream_t* st);
extern uint8 Stream_Insert(stream_t* st, uint8* data, uint32 length);
extern uint8 Stream_Get(stream_t* st, uint8* data, uint32 length);
extern uint32 Stream_Pop(stream_t* st, uint8* data, uint32 length);
extern uint8 Stream_Seek(stream_t* st, uint32 length);

#endif /* STREAM_H */
