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
#include <signal.h>
#include <sys/wait.h>

#define SOCK_PATH "usock"

int unix_server() {
    struct sockaddr_un sun, csun;
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) return sock;
    
    sun.sun_family = AF_UNIX;
    strcpy(sun.sun_path, SOCK_PATH);
    unlink(sun.sun_path);
    int len = strlen(sun.sun_path) + sizeof(sun.sun_family);
    
    if (bind(sock, (struct sockaddr *)&sun, len) == -1) {
      printf("[parent] bind failed\n");
      close(sock);
      return -1;
      
    }
    
    if (listen(sock, 5) == -1) {
      printf("[parent] listen failed\n");
      close(sock);
      return -1;
    }
    
    return sock;
}

void do_work(int csock) {
 	  struct msghdr child_msg;
	  memset(&child_msg, 0, sizeof(child_msg));
	  char cmsgbuf[CMSG_SPACE(sizeof(int))];
	  child_msg.msg_control = cmsgbuf;
	  child_msg.msg_controllen = sizeof(cmsgbuf);
	  int rc = recvmsg(csock, &child_msg, 0);
	  close(csock);
	  struct cmsghdr *cmsg = CMSG_FIRSTHDR(&child_msg);
	  int symfd;
	  memcpy(&symfd, CMSG_DATA(cmsg), sizeof(symfd));
	  size_t sz = 0; 
	  ioctl(symfd, KIOCGSIZE, &sz); 
	  sleep(1);
	  char *p = mmap(NULL, sz, PROT_READ, MAP_SHARED, symfd, 0);
	  close(symfd);
}

void fork_and_wait(int sock) {
    int pid = fork();
    if (pid == -1) return; 
    if (pid == 0) {
      do_work(sock);
      exit(0); // will never be reached 
    } else {
      close(sock);
      int status;
      waitpid(pid, &status, WEXITED);
      return; 
    }  
}

int main(int argc, char **argv) {
	int tsock = unix_server();
	while(1) {
	  struct sockaddr_un csun;
	  socklen_t t = sizeof(csun);
	  int csock = accept(tsock, (struct sockaddr *) &csun, &t);
	  if (csock == -1) {
	    close(tsock);
	    return -1;
	  }
	  fork_and_wait(csock);	      
	}
}
