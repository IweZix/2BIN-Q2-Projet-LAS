/**
 * Groupe 8
 * NICOLAS LUca
 * DE SMET Kilian
 * DIERYCK Emilien
*/

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
#include <sys/socket.h>

#include "utils_v1.h"
#include "header.h"
#include "tuile.h"
#include "socket.h"

typedef void (*childhandler_fn)(void*, void*, void*); 

/**
 * Use to pass arguments to csigint_handler
*/
struct {
  Player *shared_memory;
  int shm_id;
  int sem_id;
} handler_args;

/**
 * Use to verify if the game is finished
*/
volatile sig_atomic_t end = 0;

/**
 * Handle SIGALRM signal
*/
void sigalrm_handler(int sig) {
  printColor("\n%s\n","La période d'inscription est terminée", 33);
  end = 1;
}

/**
 * Close shared memory and semaphore
*/
void closeAll(Player *shared_memory, int shm_id, int sem_id) {
  // Detach shared memory
  sshmdt(shared_memory);
  // Destroy shared memory
  sshmdelete(shm_id);
  // Destroy semaphore
  sem_delete(sem_id);
  // reset end
  end = 0;
}

/**
 * Handle SIGINT signal
*/
void sigint_handler(int signum) {
  printColor("\n%s\n","Server is shutting down by SIGINT...", 33);
  closeAll(handler_args.shared_memory, handler_args.shm_id, handler_args.sem_id);
  exit(0);
}

/**
 * Handle child process.
*/
void child_handler(void* pipeEcriture, void* pipeLecture, void* socket) {
  int *pipefdEcriture = (int *) pipeEcriture;
  int *pipefdLecture = (int *) pipeLecture;
  int *socketfd = (int *) socket;

  StructPipeCommunication pipeCommunicationEcriture;
  StructPipeCommunication pipeCommunicationLecture;
  
  StructClientCommunication clientCommunication;

  /**
   * Game
  */
  int nbTour = 0;
  while (nbTour < 20) {
    sread(pipefdLecture[0], &pipeCommunicationLecture, sizeof(pipeCommunicationLecture));
    int tuile = pipeCommunicationLecture.tuile;

    clientCommunication.tuile = tuile;
    clientCommunication.code = ENVOIE_TUILE;
    swrite(*socketfd, &clientCommunication, sizeof(clientCommunication));

    sread(*socketfd, &clientCommunication, sizeof(clientCommunication));
    while (clientCommunication.code != RECEPTION_EMPLACEMENT) {
      sread(*socketfd, &clientCommunication, sizeof(clientCommunication));
    }

    int emplacement = clientCommunication.emplacement;

    pipeCommunicationEcriture.tuile = emplacement;
    pipeCommunicationEcriture.code = ENVOIE_PARENT;
    swrite(pipefdEcriture[1], &pipeCommunicationEcriture, sizeof(pipeCommunicationEcriture));

    nbTour++;
  }

  /**
   * Wait message to send score
  */
  sread(pipefdLecture[0], &pipeCommunicationLecture, sizeof(pipeCommunicationLecture));
  while (pipeCommunicationLecture.code != ENVOIE_SCORE) {
    sread(pipefdLecture[0], &pipeCommunicationLecture, sizeof(pipeCommunicationLecture));
  }

  /**
   * Read and send score to parent
  */
  int score;
  sread(*socketfd, &score, sizeof(int));
  pipeCommunicationEcriture.tuile = score;
  pipeCommunicationEcriture.code = ENVOIE_SCORE;
  swrite(pipefdEcriture[1], &pipeCommunicationEcriture, sizeof(pipeCommunicationEcriture));

  /**
   * Wait message to read scores
  */
  sread(pipefdLecture[0], &pipeCommunicationLecture, sizeof(pipeCommunicationLecture));
  while (pipeCommunicationLecture.code != LECTURE_SCORE) {
    sread(pipefdLecture[0], &pipeCommunicationLecture, sizeof(pipeCommunicationLecture));
  }

  /**
   * Read scores in shared memory
  */
  int sem_id = sem_create(SEM_KEY, 2, PERM, 1);
  int shmid = sshmget(KEY, MAX_PLAYER*sizeof(Player), 0);
  Player *shared_memory = sshmat(shmid);

  for (int i = 0; i < MAX_PLAYER; i++) {
    sem_down(sem_id, 0);
    if (shared_memory[i].pseudo[0] != '\0') {
      swrite(*socketfd, &shared_memory[i], sizeof(Player));
    }
    sem_up(sem_id, 0);
  }
  
  exit(0);
}



int main(int argc, char const *argv[]) {

  if (argc < 2) {
    printColor("\n%s\n", "Usage : ./server [PORT]", 31);
    return 1;
  }

  // initialisation du serveur
  int port = atoi(argv[1]);
  int sockfd = initSocketServer(port);

  // To save the current position in the file of tuiles
  long currentFilePos = 0;

  while (1) {

    // Handle signals
    ssigaction(SIGINT, sigint_handler);
    ssigaction(SIGALRM, sigalrm_handler);
  
    // Create one semaphore to store the actual piece
    int sem_id = sem_create(SEM_KEY, 2, PERM, 1);

    // Create shared memory
    int shm_id = sshmget(KEY, MAX_PLAYER*sizeof(Player), IPC_CREAT | PERM);
    // Attach shared memory
    Player *shared_memory = sshmat(shm_id);

    handler_args.shared_memory = shared_memory;
    handler_args.shm_id = shm_id;
    handler_args.sem_id = sem_id;

    StructMessage msg;
    Player players[MAX_PLAYER];
    for (int i = 0; i < MAX_PLAYER; i++) {
      shared_memory[i] = players[i];
    }
    int nbPlayer = 0;
    
    struct pollfd fds[MAX_PLAYER];

    /**
     * Inscription
    */
    alarm(10);
    while (!end) {

      int newsockfd = accept(sockfd, NULL, NULL);
      if (newsockfd > 0) {

        sread(newsockfd, &msg, sizeof(msg));

        char buffer[200] = "Inscription demandée par le joueur ";
        strcat(buffer, msg.message);
        printColor("\n%s\n", buffer, 32);

        if (nbPlayer < MAX_PLAYER) {
          strcpy(players[nbPlayer].pseudo, msg.message);
          players[nbPlayer].socketfd = newsockfd;
          players[nbPlayer].score = -1;

          shared_memory[nbPlayer] = players[nbPlayer];

          fds[nbPlayer].fd = newsockfd;
          fds[nbPlayer].events = POLLIN;

          nbPlayer++;

          msg.code = INSCRIPTION_OK;
          swrite(newsockfd, &msg, sizeof(msg));

          printColor("\n%s\n", "Inscription réussie", 32);
        } else {
          msg.code = INSCRIPTION_KO;
          swrite(newsockfd, &msg, sizeof(msg));

          printColor("\n%s\n", "Inscription échouée", 31);
        }
        
        printf("Nb Inscriptions : %i\n", nbPlayer);
      }
    }

    if (nbPlayer < 2) {
      printColor("\n%s\n", "Il n'y a pas assez de joueurs pour lancer la partie", 31);
      for (int i = 0; i < nbPlayer; i++) {
        sclose(players[i].socketfd);
      }
      close(sockfd);
      closeAll(shared_memory, shm_id, sem_id);
      return 0;
    }
    
    for (int i = 0; i < nbPlayer; i++) {
      printf("%d\n", fds[i].fd);
    }

    /**
     * Tuiles generation
    */
    int *tuiles;
    if (argc > 2) {
      FILE *file = fopen(argv[2], "r");
      if (file == NULL) {
        printColor("\n%s\n", "Erreur lors de l'ouverture du fichier", 31);
        return 1;
      }
      tuiles = malloc(20 * sizeof(int));
      fseek(file, currentFilePos, SEEK_SET);

      for (int i = 0; i < 20; i++) {
        fscanf(file, "%d", &tuiles[i]);
      }

      currentFilePos = ftell(file);

      fclose(file);
    } else {
      tuiles = genTuile();
    }
    

    /**
     * Pipe creation
    */
    int *childTab = malloc(nbPlayer * sizeof(pid_t));

    struct pollfd *fdsChildLecture = malloc(nbPlayer * sizeof(struct pollfd));
    struct pollfd *fdsChildEcriture = malloc(nbPlayer * sizeof(struct pollfd));

    for (int i = 0; i < nbPlayer; i++) {
      int pipefdLecture[2];
      spipe(pipefdLecture);
      int pipefdEcriture[2];
      spipe(pipefdEcriture);

      pid_t childId = fork_and_run3((childhandler_fn) child_handler, pipefdLecture, pipefdEcriture, &players[i].socketfd);
      childTab[i] = childId;

      fdsChildLecture[i].fd = pipefdLecture[0];
      fdsChildLecture[i].events = POLLIN;

      fdsChildEcriture[i].fd = pipefdEcriture[1];
      fdsChildEcriture[i].events = POLLOUT;
    }

    /**
     * Game
    */
    int tour = 1;
    StructPipeCommunication pipeCommunication;

    while (tour < 21) {
      int tuile = tuiles[tour-1];

      for (int i = 0; i < nbPlayer; i++) {
        pipeCommunication.tuile = tuile;
        pipeCommunication.code = ENVOIE_PIPE;
        swrite(fdsChildEcriture[i].fd, &pipeCommunication, sizeof(pipeCommunication));
      }

      int nbJoueurAJouer = 0;
      while (nbJoueurAJouer < nbPlayer) {
        int ret = spoll(fdsChildLecture, nbPlayer, 0);
        if (ret > 0) {
          for (int i = 0; i < nbPlayer; i++) {
            if (fdsChildLecture[i].revents & POLLIN) {
              sread(fdsChildLecture[i].fd, &pipeCommunication, sizeof(pipeCommunication));
              nbJoueurAJouer++;
            }
          }
        }
      }

      tour++;
    }

    printf("Fin du jeu\n");

    /**
     * Send message to children to send their score
    */
    for (int i = 0; i < nbPlayer; i++) {
      pipeCommunication.tuile = 0;
      pipeCommunication.code = ENVOIE_SCORE;
      swrite(fdsChildEcriture[i].fd, &pipeCommunication, sizeof(pipeCommunication));
    }

    /**
     * Wait for children to send their score
    */
    int nbPlayerScore = 0;
    StructPipeCommunication pipeCommunicationScore;
    while (nbPlayerScore < nbPlayer) {
      int ret = spoll(fdsChildLecture, nbPlayer, 0);
      if (ret > 0) {
        for (int i = 0; i < nbPlayer; i++) {
          if (fdsChildLecture[i].revents & POLLIN) {
            sread(fdsChildLecture[i].fd, &pipeCommunicationScore, sizeof(pipeCommunicationScore));
            players[i].score = pipeCommunicationScore.tuile;
            nbPlayerScore++;
          }
        }
      }
    }

    /**
     * Sort players by score
    */
    for (int i = 0; i < nbPlayer; i++) {
      for (int j = i+1; j < nbPlayer; j++) {
        if (players[i].score < players[j].score) {
          Player temp = players[i];
          players[i] = players[j];
          players[j] = temp;
        }
      }
    }

    for (int i = nbPlayer; i < MAX_PLAYER; i++) {
      players[i].pseudo[0] = '\0';
    }

    // down semaphore
    sem_down(sem_id, 0);

    /**
     * Update shared memory
    */
    for (int i = 0; i < MAX_PLAYER; i++) {
      shared_memory[i] = players[i];
    }

    // up semaphore
    sem_up(sem_id, 0);

    /**
     * Send message to children to read scores
    */
    for (int i = 0; i < nbPlayer; i++) {
      pipeCommunication.tuile = 0;
      pipeCommunication.code = LECTURE_SCORE;
      swrite(fdsChildEcriture[i].fd, &pipeCommunication, sizeof(pipeCommunication));
    }
    
    // attendre les enfants
    for (int i = 0; i < nbPlayer; i++) {
      swaitpid(childTab[i], NULL, 0);
    }

    end = 0;

    for (int i = 0; i < nbPlayer; i++) {
      sclose(players[i].socketfd);
    }
  
    closeAll(shared_memory, shm_id, sem_id);
  }

  sclose(sockfd);
  return 0;
}