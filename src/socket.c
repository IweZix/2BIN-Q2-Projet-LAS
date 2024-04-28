#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "utils_v1.h"
#include "socket.h"
#include "header.h"

int initSocketServer(int port) {
  int sockfd = ssocket();
  sbind(port, sockfd);
  slisten(sockfd, 5);

  char buffer[200] = "Le serveur est en écoute sur le port ";
  char portStr[10];
  sprintf(portStr, "%d", port);
  strcat(buffer, portStr);
  printColor("\n%s\n", buffer, 32);

  return sockfd;
}

int initSocketClient(int port) {
  int sockfd = ssocket();
  sconnect(SERVER_IP, port, sockfd);

  char buffer[200] = "Le client est connecté sur le port ";
  char portStr[10];
  sprintf(portStr, "%d", port);
  strcat(buffer, portStr);
  printColor("\n%s\n", buffer, 32);

  return sockfd;
}