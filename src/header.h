#ifndef _HEADER_H_
#define _HEADER_H_

// Shared memory
#define KEY 100
#define SEM_KEY 200
#define PERM 0666

// Socket
#define SERVER_PORT 9000
#define SERVER_IP "127.0.0.1"
#define BACKLOG 5

// define max player
#define MAX_PLAYER 5

// Struct Player
typedef struct Player {
  char pseudo[100];
  int socketfd;
  int score;
} Player;

// enum Code
enum Code {
  INSCRIPTION_REQUEST = 1,
  INSCRIPTION_OK = 2,
  INSCRIPTION_KO = 3,

  // Communication
  ENVOIE_TUILE = 4,
  RECEPTION_EMPLACEMENT = 5,

  // Pipe
  ENVOIE_PIPE = 6,
  ENVOIE_PARENT = 7,

  ENVOIE_SCORE = 9,
  LECTURE_SCORE = 10,

  // End
  FIN = 8
};

// Struct Message
typedef struct StructMessage {
  char message[100];
  int code;
} StructMessage;

typedef struct StructClientCommunication {
  int tuile;
  int emplacement;
  int code;
} StructClientCommunication;

typedef struct StructPipeCommunication {
  int tuile;
  int code;
} StructPipeCommunication;


#endif // _HEADER_H_