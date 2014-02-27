#include "confutils.h"

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/time.h>
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h>
#include <time.h> 
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXMSGLEN  1024

static void build_set(int *sock, fd_set *readfds){
	FD_ZERO(readfds);
	FD_SET(*sock, readfds);
	FD_SET(STDIN_FILENO, readfds);
}		

static void service_server(int *sock){
	char *msg = recvtext(*sock);
	if (!msg) { 
			fprintf(stderr, "error: server died\n");
			exit(1);
	}
	printf(">>> %s", msg);
	free(msg);
}

static void service_user(int *sock){
	char msg[MAXMSGLEN];
	if (!fgets(msg, sizeof msg, stdin)) {
			close(*sock);
	}
	sendtext(*sock, msg);
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
			fprintf(stderr, "usage : %s <servhost> <servport>\n", argv[0]);
			exit(1);
	}

	int sock = hook_to_server(argv[1], argv[2]);
	if (sock == -1) exit(1);
	fd_set readfds;

	while (1) {
			build_set(&sock, &readfds);
			if (select(sock + 1, &readfds, NULL, NULL, NULL) < 0 ){
					perror("select error");
					exit(1);
			}
			if (FD_ISSET(sock, &readfds)){
					service_server(&sock);
			}
			if (FD_ISSET(STDIN_FILENO, &readfds)){
					service_user(&sock);
			}
	}

}
