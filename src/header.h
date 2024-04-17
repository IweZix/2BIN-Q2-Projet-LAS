#ifndef _HEADER_H_
#define _HEADER_H_

// Shared memory
#define KEY 100
#define SEM_KEY 200
#define PERM 0666

// Socket
#define SERVER_PORT 8080
#define SERVER_IP "127.0.0.1"
#define BACKLOG 5

// define max player
#define MAX_PLAYER 5

// Struct Player
typedef struct Player {
  char pseudo[50];
  int socketfd;
  int score;
} Player;

// enum Code
enum Code {
  INSCRIPTION_REQUEST = 1,
  INSCRIPTION_OK = 2,
  INSCRIPTION_KO = 3,
};

// Struct Message
typedef struct StructMessage {
  char message[100];
  int code;
} StructMessage;

#endif // _HEADER_H_