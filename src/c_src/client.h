#ifndef CLIENT_H
#define CLIENT_H

/* DEFINITIONS */
#define HEADER_LENGTH 15


/* FUNCTIONS */
int init_sock(const char* address, int port, int retry);
int send_msg(int sockfd, char* msg, int length);
int recv_msg(int sockfd, char** buffer);
int shutdown_sock(int sockfd);


#endif // CLIENT_H
