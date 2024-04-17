#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#include "utils_v1.h"
#include "header.h"

int initSocketClient() {
  int sockfd = ssocket();
  sconnect(SERVER_IP, SERVER_PORT, sockfd);

  char buffer[200] = "Le client est connecté sur le port ";
  char port[10];
  sprintf(port, "%d", SERVER_PORT);
  strcat(buffer, port);
  printColor("\n%s\n", buffer, 32);

  return sockfd;
}

int main(int argc, char const *argv[]) {

  printf("Bienvenue dans le programe d'inscription au serveur de jeu\n");
  printf("Pour participer entrez votre nom :\n");
  StructMessage msg;
  int ret = sread(0, msg.message, 100);
  msg.message[ret - 1] = '\0';
  msg.code = INSCRIPTION_REQUEST;

  int sockfd = initSocketClient();
  swrite(sockfd, &msg, sizeof(msg));

  /* wait server response */
  sread(sockfd, &msg, sizeof(msg));

  if (msg.code == INSCRIPTION_OK) {
    printColor("\n%s\n", "Inscription réussie", 32);
  } else {
    printColor("\n%s\n", "Inscription échouée", 31);
  }



  
  
  return 0;
}
