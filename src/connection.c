#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "stream.h"
#include <stdlib.h>

static int port = 2220;
static char server_url[16];
static int sock;

void connection_setServer(char* url, int p) {
	char safetyBuf[16];
	memcpy(&safetyBuf[0], url, 15);
	safetyBuf[15] = 0;
	strcpy(&server_url[0], safetyBuf);
	port = p;
}

int connection_connect()
{
    sock = 0;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, server_url, &serv_addr.sin_addr)<=0)
    {
        printf("Invalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Connection Failed \n");
        return -1;
    }

    return sock;
}

uint32 connection_getTime(void) {
	sint8 buf[14];
	buf[13] = 0;

	struct timespec tstart={0,0}, tend={0,0};
	clock_gettime(CLOCK_MONOTONIC, &tstart);

	send(sock , "TIME\n" , 5 , 0 );
	uint8 num = read(sock, buf, 13);

	clock_gettime(CLOCK_MONOTONIC, &tend);

	uint16 time = 1000*tend.tv_sec + (tend.tv_nsec / 1000000) -
		    1000*tstart.tv_sec - (tstart.tv_nsec / 1000000);

	//printf("sending took about %d milliseconds\n", time);

	if (num != 13) {
		return 0;
	}

	return atol(&buf[0]) - (time / 2);
}

void connection_close() {
	close(sock);
}

