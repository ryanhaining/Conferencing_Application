/*--------------------------------------------------------------------*/
/* conference server */

#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h>
#include <time.h> 
#include <errno.h>
#include <sys/select.h>
#include <stdlib.h>
#include <unistd.h>

char *recvtext(int sd);
int sendtext(int sd, char *msg);
int start_server();
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/

typedef struct Clients{
	int client_list[1024];
	int size;
}Clients;

typedef struct LiveClients{
	int livesdmax;
	fd_set livesdset;
}LiveClients;

typedef struct HostInfo{
	char client_host[128];
	char client_port_nb[128];
}HostInfo; 

void build_set(Clients *clients, LiveClients *live_clients, int* servsock){
	live_clients->livesdmax = *servsock;
	FD_ZERO(&(live_clients->livesdset));
	FD_SET(*servsock, &(live_clients->livesdset));
	for (int i = 0; i < clients->size; i++){
		if (clients->client_list[i] > live_clients->livesdmax){
			live_clients->livesdmax = clients->client_list[i];
		}
		FD_SET(clients->client_list[i], &(live_clients->livesdset));
	}	
}
struct sockaddr_in get_client_info(int *frsock){
	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(sockaddr));
	socklen_t len = sizeof(sockaddr);
	if (getpeername(*frsock, (struct sockaddr*)&sockaddr, &len) < 0){
		perror("getpeername() error");
		exit(1);	
	}
	return sockaddr;
}

HostInfo get_host_info(struct sockaddr_in *sockaddr){		
	HostInfo host_info;
	if (getnameinfo((struct sockaddr*)sockaddr, sizeof(*sockaddr), host_info.client_host, sizeof(host_info.client_host),
		host_info.client_port_nb, sizeof(host_info.client_port_nb), 0) != 0){
			perror("getnameinfo() error");
			exit(1);	
	}
	return host_info;
}
void disconnect(Clients *clients, int *frsock){
	struct sockaddr_in sockaddr = get_client_info(frsock);
	HostInfo host_info = get_host_info(&sockaddr);	
	unsigned short client_port = ntohs(sockaddr.sin_port);
	printf("admin: disconnect though from '%s(%hu)'\n", host_info.client_host, client_port);
	
	for (int i = 0 ; i < clients->size; i++){
		if (clients->client_list[i] == *frsock){
			close(clients->client_list[i]);
			clients->client_list[i] = clients->client_list[clients->size -1];		
			--clients->size;
			clients->client_list[clients->size] = 0;	
			break;
		}
	}
	close(*frsock);
}
void relay_message(Clients *clients, int *frsock, char *msg){
	struct sockaddr_in sockaddr = get_client_info(frsock);
	HostInfo host_info = get_host_info(&sockaddr);	
	unsigned short client_port = ntohs(sockaddr.sin_port);
	printf("%s(%hu): %s", host_info.client_host, client_port, msg);
	for (int i = 0; i < clients->size; i ++){
		if (clients->client_list[i] != 0 && clients->client_list[i] != *frsock){
			if (sendtext(clients->client_list[i], msg) < 0){
				fprintf(stderr, "Error writing to %d\n", clients->client_list[i]);
				exit(1);
			}
		}			
	}
}
void accept_connection(Clients* clients, int *servsock){
	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(sockaddr));
	socklen_t len = sizeof(sockaddr);	
            
	int csd = accept(*servsock, (struct sockaddr*)&sockaddr, &len);
	if (csd != -1) {
		HostInfo host_info= get_host_info(&sockaddr);
		unsigned short client_port = ntohs(sockaddr.sin_port);
		printf("admin: connect from '%s' at '%hu'\n", host_info.client_host, client_port);
		clients->client_list[clients->size++] = csd;
	}else{
		perror("accept() error");
		exit(1);
	}
}
int main(int argc, char *argv[]){
	/* check usage */
	if (argc != 1) {
		fprintf(stderr, "usage : %s\n", argv[0]);
		exit(1);
	}
	/* get ready to receive requests */
	int servsock = start_server();
	if (servsock == -1) {
		exit(1);
	}
	Clients clients = {{0}, 0};
	LiveClients live_clients;
 
	while (1) {
		build_set(&clients, &live_clients, &servsock);
		if (select(live_clients.livesdmax + 1, &live_clients.livesdset, NULL, NULL, NULL) < 0) {
			perror("select() error");
			exit(1);
		}
		for (int client_num = 0; client_num < clients.size; client_num++){
			int frsock = clients.client_list[client_num];
			if (frsock == servsock) continue;
 
			if (FD_ISSET(frsock, &live_clients.livesdset)){
				char *msg = recvtext(frsock);
				if (!msg){
					disconnect(&clients, &frsock);
				}
				else {
					relay_message(&clients, &frsock, msg);
				}	
                free(msg);
			}
		}
        if (FD_ISSET(servsock, &live_clients.livesdset)){
			accept_connection(&clients, &servsock);		
		}
	}
}
/*--------------------------------------------------------------------*/
