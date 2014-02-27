#ifndef CONFUTILS__H__
#define CONFUTILS__H__

char *recvtext(int sd);
int sendtext(int sd, const char *msg);
int start_server();
int hook_to_server(const char *servhost, const char* servport);

#endif // #ifndef CONFUTILS__H__
