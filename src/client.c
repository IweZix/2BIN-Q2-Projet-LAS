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

void printPlateau(int *plateau) {
  for (int i = 0; i < 20; i++) {
    printf("%d ", plateau[i]);
  }
  printf("\n");
}

int calculeScore(int plateau[20]) {
    int nb_suites = 0; 
    int suites_par_longueur[21] = {0};

    int i = 0;
    
    while (i < 20) {

        int debut_suite = i;

        while (i < 19 && (plateau[i] <= plateau[i+1] || (plateau[i] == 31 && plateau[i+1] > plateau[i-1]) || plateau[i+1] == 31)) {
            i++;
        }

        int fin_suite = i;

        int longueur_suite = fin_suite - debut_suite + 1;

        if (longueur_suite >= 2) {
            nb_suites++;
            suites_par_longueur[longueur_suite]++;
        }
        i++;
    }


    int scores[] = {0, 1, 3, 5, 7, 9, 11, 15, 20, 25, 30, 35, 40, 50, 60, 70, 85, 100, 150, 300};


    int score_total = 0;
    for (int k = 2; k < 21; k++) {
        score_total += suites_par_longueur[k] * scores[k-1];
    }

    return score_total;
}

int main(int argc, char const *argv[]) {

  printf("Bienvenue dans le programe d'inscription au serveur de jeu\n");
  printf("Pour participer entrez votre nom :\n");
  StructMessage msg;
  fgets(msg.message, sizeof(msg.message), stdin);
  //msg.message[ret - 1] = '\0';
  msg.message[strlen(msg.message) - 1] = '\0';
  msg.code = INSCRIPTION_REQUEST;
  printf("Votre nom est : %s\n", msg.message);

  int port = atoi(argv[1]);
  int sockfd = initSocketClient(port);
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
    while (tuileCommunication.code != ENVOIE_TUILE) {
      sread(sockfd, &tuileCommunication, sizeof(tuileCommunication));
    }
    
    printColor("\n%s\n", "Voici le plateau actuel", 32);
    printPlateau(plateau);
    printColor("\n%s", "Voici la tuile que vous avez reçu : ", 32);
    printf("%d\n", tuileCommunication.tuile);

    int position;
    printColor("\n%s", "Entrez la position où vous voulez placer la tuile : ", 32);
    scanf("%d", &position);
    position = position-1;

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
    tuileCommunication.code = RECEPTION_EMPLACEMENT;

    swrite(sockfd, &tuileCommunication, sizeof(tuileCommunication));
  }

  printColor("\n%s\n", "Partie terminée", 32);

  printColor("\n%s\n", "Voici le plateau final", 32);
  printPlateau(plateau);

  int score = calculeScore(plateau);
  printf("Votre score est de %d\n", score);
  
  swrite(sockfd, &score, sizeof(int));

  // lecture du tableau des scores dans le socket
  Player players[MAX_PLAYER];
  for (int i = 0; i < MAX_PLAYER; i++) {
    sread(sockfd, &players[i], sizeof(Player));
  }
  
  printColor("\n%s\n", "Voici le tableau des scores", 32);
  for (int i = 0; i < MAX_PLAYER; i++) {
      if (players[i].pseudo[0] != '\0' && players[i].score > 0 && players[i].socketfd > 0) {
          printf("%s : %d\n", players[i].pseudo, players[i].score);
      }
  }


  
  close(sockfd);
  return 0;
}
