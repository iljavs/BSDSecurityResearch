#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <sys/ksyms.h>

#define SOCK_PATH "usock"

int unix_client() {
    struct sockaddr_un sun;
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) return sock;
    
    sun.sun_family = AF_UNIX;
    strcpy(sun.sun_path, SOCK_PATH);
    unlink(sun.sun_path);
    int len = strlen(sun.sun_path) + sizeof(sun.sun_family);
  
    if (connect(sock, (struct sockaddr *)&sun, len) == -1) {
      printf("[child] connect() failed\n");
      close(sock);
      return -1;      
    }
    
    return sock;    
}

int main(int argc, char **argv) {
	int fd = open("/dev/ksyms", O_RDONLY);
	if (fd == -1) {
		printf("[child] /dev/ksyms open failed\n");
		exit(0);
	}
	sleep(1); // give server a chance to start
	int sock = unix_client();
	if (sock == -1) {
	  printf("[child] couldn't create unix socket\n");
	  exit(0);
	  
	}
	
	struct msghdr parent_msg; 
	memset(&parent_msg, 0, sizeof(parent_msg));
	char cmsgbuf[CMSG_SPACE(sizeof(fd))];
	parent_msg.msg_control = cmsgbuf;
	parent_msg.msg_controllen = sizeof(cmsgbuf);
	struct cmsghdr *cmsg = CMSG_FIRSTHDR(&parent_msg);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(fd));
	memcpy(CMSG_DATA(cmsg), &fd, sizeof(fd));
	parent_msg.msg_controllen = cmsg->cmsg_len;

	if (sendmsg(sock, &parent_msg, 0) < 0) {
		printf("[child] sendmsg() failed\n");
		perror("[child] sendmsg():");
		exit(0);
	}
	printf("[child] succesfully send fd to other process\n");
	exit(0);

}
