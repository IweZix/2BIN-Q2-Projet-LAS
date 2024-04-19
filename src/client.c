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

void printPlateau(int *plateau) {
  for (int i = 0; i < 20; i++) {
    printf("%d ", plateau[i]);
  }
  printf("\n");
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

  StructClientCommunication tuileCommunication;

  int plateau[20] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  };

  int nbTuiles = 0;

  while (nbTuiles < 20) {
    sread(sockfd, &tuileCommunication, sizeof(tuileCommunication));
    while (tuileCommunication.code != TUILE) {
      sread(sockfd, &tuileCommunication, sizeof(tuileCommunication));
    }
    
    printColor("\n%s\n", "Voici le plateau actuel", 32);
    printPlateau(plateau);
    printColor("\n%s", "Voici la tuile que vous avez reçu : ", 32);
    printf("%d\n", tuileCommunication.tuile);

    int position;
    printColor("\n%s", "Entrez la position où vous voulez placer la tuile : ", 32);
    scanf("%d", &position);

    if (position < 0 || position > 19) {
      printColor("\n%s\n", "Position invalide", 31);
      continue;
    }

    while (plateau[position] != 0) {
      if (position == 19) {
        position = 0;
      } else {
        position++;
      }
    }

    plateau[position] = tuileCommunication.tuile;
    
    nbTuiles++;

    tuileCommunication.emplacement = position;
    tuileCommunication.code = EMPLACEMENT;

    swrite(sockfd, &tuileCommunication, sizeof(tuileCommunication));
  }

  printColor("\n%s\n", "Partie terminée", 32);

  printColor("\n%s\n", "Voici le plateau final", 32);
  printPlateau(plateau);

  printColor("\n%s", "Entrez votre score : ", 32);
  int score;
  scanf("%d", &score);
  

  

  
  close(sockfd);
  return 0;
}
