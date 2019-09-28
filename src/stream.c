/*================================================================================*
 * Includes
 *================================================================================*/

#include "types.h"
#include "stream.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

/*================================================================================*
 * Defines
 *================================================================================*/

/*================================================================================*
 * Variables
 *================================================================================*/

/*================================================================================*
 * Prototypes
 *================================================================================*/

/*================================================================================*
 * Functions
 *================================================================================*/

void Stream_Init(stream_t* st, uint32 length, uint32 startPosition, uint32 spm) {
	st->next = 0u;
	st->last = 0u;
	st->full= FALSE;
	st->length = length;
	st->data = (uint8*) malloc(length);
	st->position = startPosition * spm;
	st->stepsPerTenMs = spm;
	pthread_mutex_init(&st->mutex, NULL);
}

void Stream_Close(stream_t* st) {
	free(st->data);
}

void Stream_SetPosition(stream_t* st, uint32 pos) {
	st->position = pos * st->stepsPerTenMs * 4u / 10u;
}

uint32 Stream_Length(stream_t* st) {
	//printf("Rest ist %d length ist %d\n",Stream_GetRest(st), st->length);

	return st->length - Stream_GetRest(st);
}

uint32 Stream_GetRest(stream_t* st) {
	if (TRUE == st->full) {
		return 0u;
	}

	return (((st->length - st->last) + st->next - 1) % st->length) + 1;
}

uint8 Stream_PutElement(stream_t* st, uint8* data, uint32 length) {
	if (length + 4 > Stream_GetRest(st)) {
		return E_NOT_OK;
	}

	Stream_Insert(st, (uint8*) &length, 4);
	Stream_Insert(st, data, length);

	return E_OK;
}

static void Stream_Resize(stream_t* st, uint32 length) {
	uint8* data = (uint8*) malloc(st->length*2 + 2*length);
	uint32 oldDataLength = Stream_Length(st);

	Stream_Get(st, data, oldDataLength);

	free(st->data);

	st->next = 0u;
	st->last = oldDataLength;
	st->full= FALSE;
	st->length = st->length*2 + 2*length;
	st->data = data;

	printf("Size of stream to %d length was %d\n", st->length, oldDataLength);
}

uint8 Stream_Insert(stream_t* st, uint8* data, uint32 length) {

	pthread_mutex_lock(&st->mutex);
	if (Stream_GetRest(st) < length) {
		Stream_Resize(st,length);
	}

	uint32 restToEnd = st->length - st->last;

	if (restToEnd >= length) {
		memcpy(st->data + st->last, data, length);
		st->last += length;
	} else {
		memcpy(st->data + st->last, data, restToEnd);
		memcpy(st->data, data + restToEnd, length - restToEnd);
		st->last = length - restToEnd;
		printf("OVERLAP INPUT!\n");
	}

	if (st->last == st->next) {
		st->full = TRUE;
	}

	pthread_mutex_unlock(&st->mutex);

	return E_OK;
}

uint32 Stream_Poll(stream_t* st, uint8* data, uint8 remove) {

	uint32 length;
	uint32 saveNext = st->next;

	Stream_Pop(st, (uint8*) &length, 4u);
	Stream_Pop(st, data, length);

	if (FALSE == remove) {
		st->next = saveNext;
	} else {
		st->full = FALSE;
	}

	return length;
}

uint32 Stream_LengthTop(stream_t* st) {

	uint32 length;

	Stream_Get(st, (uint8*) &length, 4u);

	return length;
}

uint8 Stream_Rewind(stream_t* st, uint32 length) {

	uint32 restToStart = st->next;

	if (restToStart >= length) {
		st->next -= length;
	} else {
		st->next = st->length - (length - restToStart);
	}

	//printf("Rewind -= %d\n", length);
	st->position -= length;

	//printf("Stream at position: %d:%2d\n", st->position/44100/60/8, (st->position/44100/8) % 60);

	return E_OK;
}

uint8 Stream_Seek(stream_t* st, uint32 length) {

	uint32 restToEnd = st->length - st->next;

	if (restToEnd >= length) {
		st->next += length;
	} else {
		st->next = length - restToEnd;
	}

	if (length > 0) {
		st->full = FALSE;
	}

	//printf("Seek += %d\n", length);
	st->position += length;

	//printf("Stream at position: %d:%2d\n", st->position/44100/60/8, (st->position/44100/8) % 60);

	return E_OK;
}

uint32 Stream_Pop(stream_t* st, uint8* data, uint32 length) {

	uint32 restToEnd = st->length - st->next;
	uint32 maxData = Stream_Length(st);

	if (maxData < length) {
		length = maxData;
	}


	if (restToEnd >= length) {
		memcpy(data, st->data + st->next, length);
		pthread_mutex_lock(&st->mutex);
		st->next += length;
	} else {
		memcpy(data, st->data + st->next, restToEnd);
		memcpy(data + restToEnd, st->data, length - restToEnd);
		pthread_mutex_lock(&st->mutex);
		st->next = length - restToEnd;
	}

	st->position += length;

	printf("Stream at position: %d:%2d\n", st->position/44100/60/8, (st->position/44100/8) % 60);

	if (length > 0) {
		st->full = FALSE;
	}
	pthread_mutex_unlock(&st->mutex);

	return length;
}

uint32 Stream_Get(stream_t* st, uint8* data, uint32 length) {

	uint32 restToEnd = st->length - st->next;
	uint32 maxData = Stream_Length(st);

	if (maxData < length) {
		//printf("Nur %d bytes zu holen.\n",maxData);
		length = maxData;
	}

	if (restToEnd >= length) {
		memcpy(data, st->data + st->next, length);
	} else {
		memcpy(data, st->data + st->next, restToEnd);
		memcpy(data + restToEnd, st->data, length - restToEnd);
	}

	return length;
}

void Stream_Print(stream_t* st) {
	uint32 count;

	for (count = 0u; count < st->length; count++) {
		printf("%02x ",*(st->data+count));
	}

	printf("\n");

	for (count = 0u; count < st->length; count++) {
		if (count == st->next && count == st->last) {
					printf("nl ");
		} else if (count == st->next) {
			printf(" n ");
		} else if (count == st->last) {
			printf(" l ");
		} else {
			printf("   ");
		}

	}
	printf("\n");
}
