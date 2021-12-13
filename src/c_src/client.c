#include <arpa/inet.h>
#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "client.h"
#include "msg.h"


/* Prototypes */
int init_sock(const char* address, int port, int retry);
int send_msg(int sockfd, char* msg, int length);
int recv_msg(int sockfd, char** buffer);
int shutdown_sock(int sockfd);


/* Functions */
int init_sock(const char* address, int port, int retry)
{
  int sockfd = 0;
  struct sockaddr_in serv_addr;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("Socket creation failed\n");
    return 0;
  }

	// server.sin_addr.s_addr = inet_addr(address);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  if (inet_pton(AF_INET, address, &serv_addr.sin_addr) <= 0)
  {
    printf("Invalid address/Address not supported\n");
    return 0;
  }

  while (retry)
  {
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
      printf("Successfully connected to server.\n");
      break;
    }
    retry--;
    sleep(5);
  }

  if (!retry)
  {
    close(sockfd);
    printf("Connection failed\n");
    return 0;
  }

  return sockfd;
}

int send_msg(int sockfd, char* msg, int length)
{
  char header[HEADER_LENGTH] = {'\0'};
  sprintf(header, "%d", length);

  // Send the header containing the length of the message
  // and then the payload
  if (send(sockfd, header, HEADER_LENGTH, 0) == -1 ||
      send(sockfd, msg, length, 0) == -1)
  {
    printf("Send failed\n");
    return 0;
  }

  return 1;
}

int recv_msg(int sockfd, char** buffer)
{
  char header[HEADER_LENGTH+1] = {'\0'};
  int msg_len;

  // Receive the header
  if (recv(sockfd, header, HEADER_LENGTH, 0) == -1)
  {
    printf("Recv failed\n");
    return 0;
  }

  msg_len = (int)strtol(header, NULL, 10);
  *buffer = (char*)malloc(sizeof(char)*(msg_len+1));

  // Receive the message
  if (recv(sockfd, *buffer, msg_len, 0) == -1)
  {
    printf("Recv failed\n");
    return 0;
  }

  return msg_len;
}

int shutdown_sock(int sockfd)
{
  json_object* msg = json_object_new_object();
  json_object_object_add(msg, "op", json_object_new_string(QUIT));
  char* msg_str = (char*)json_object_to_json_string(msg);

  if (send_msg(sockfd, msg_str, strlen(msg_str)))
    return 0;

  close(sockfd);
  json_object_put(msg);
  return 1;
}
