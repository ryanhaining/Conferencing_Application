/*--------------------------------------------------------------------*/
/* functions to connect clients and server */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h>
#include <time.h> 
#include <errno.h>

#define MAXNAMELEN 256
/*--------------------------------------------------------------------*/


/*----------------------------------------------------------------*/
void get_server_info(int *sd){
	char servhost[128];
	unsigned short servport;
	struct sockaddr_in sockaddr;
	socklen_t addrlen = sizeof(sockaddr);
	memset(&sockaddr, 0, sizeof(sockaddr));

	if (gethostname(servhost, sizeof(servhost)) < 0){
		perror("gethostname() error");
		exit(1);
	}
	if (getsockname(*sd, (struct sockaddr *)&sockaddr, &addrlen) < 0){
		perror("getsockname() error");	
		exit(1);
	}
	servport = ntohs(sockaddr.sin_port);
	printf("admin: started server on '%s' at '%hu'\n", servhost, servport);

}
void get_connection_info(const char *servhost, const char *servport, int *sd){
	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(sockaddr));
	socklen_t len = sizeof(sockaddr);
	
	if (getsockname(*sd, (struct sockaddr *)&sockaddr, &len) < 0){
		perror("getsockname() error");
		exit(1);
	}	
	int clientport = ntohs(sockaddr.sin_port);
  	printf("admin: connected to server on '%s' at '%s' thru '%d'\n", servhost, servport, clientport);
}
int start_server(){
	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(sockaddr));
	
	int sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd < 0){
		return -1;
	}
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0){
		perror("bind() error");
		return -1;
	}	
 	/* we are ready to receive connections */
  	if (listen(sd, 5) < 0) {
		perror("listen() error");
		return -1;
	}
	get_server_info(&sd);
	return sd;
}
/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
int hook_to_server(const char *servhost, const char* servport){
	int sd;
 	struct addrinfo hints;
	struct addrinfo *result;
	memset(&hints, 0, sizeof(hints));

	sd = socket(PF_INET, SOCK_STREAM, 0);
	if (sd < 0){
		return -1;
	}
	hints.ai_family = AF_INET;
	if (getaddrinfo(servhost, servport, &hints, &result) != 0){
		return -1;
	}
	struct addrinfo *addrinfo;	
	for (addrinfo = result; addrinfo != NULL; addrinfo = addrinfo->ai_next){		
		if (addrinfo->ai_family == AF_INET){
			struct sockaddr_in *serv = (struct sockaddr_in *)addrinfo->ai_addr;
			if (connect(sd, (struct sockaddr *)serv, sizeof *serv) < 0){
				perror("connect() error");
				return -1;
			}
			break;
		}
	}
	get_connection_info(servhost, servport, &sd);
	freeaddrinfo(result);
	return sd;
}
/*----------------------------------------------------------------*/

/* The following code is written by Yao Liu. */

/*----------------------------------------------------------------*/
int readn(int sd, char *buf, int n){
	int toberead;
	char *ptr;
	toberead = n;
	ptr = buf;
	while (toberead > 0){
		int byteread;
		byteread = read(sd, ptr, toberead);
		if (byteread <= 0){
			if (byteread == -1)
				perror("read() error");
			return 0;
		}
		toberead -= byteread;
		ptr += byteread;
	}
	return 1;
}
char *recvtext(int sd){
	char *msg;
	long  len;
	/* read the message length */
	if (!readn(sd, (char *) &len, sizeof(len))){
		return NULL;
	}
	len = ntohl(len);
	/* allocate space for message text */
	msg = NULL;
	if (len > 0){
		msg = (char *) malloc(len);
		if (!msg) {
			fprintf(stderr, "error : unable to malloc\n");
			return NULL;
		}
		/* read the message text */
		if (!readn(sd, msg, len)){
			free(msg);
			return NULL;
		}
	}
  /* done reading */
  return msg ;
}

int sendtext(int sd, char *msg){
	long len;
	/* write lent */
	len = (msg ? strlen(msg) + 1 : 0);
	len = htonl(len);
	if (write(sd, (char *) &len, sizeof(len)) < 0) return -1;
		
	/* write message text */
	len = ntohl(len);
	if (len > 0){
		if (write(sd, msg, len) < 0) return -1;
	}
	return 1;
}
/*----------------------------------------------------------------*/
