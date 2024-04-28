#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "utils_v1.h"

/**
 * Initialize the server socket
*/
int initSocketServer(int port);

#endif
