
#ifndef CLIENT_H
#define CLIENT_H

#include <signal.h>
#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>  /* define socket */ 
#include <netinet/in.h>  /* define internet socket */ 
#include <netdb.h>       /* define internet socket */ 
 
#define SERVER_PORT 4747     /* define a server port number */
#define BUFFER_LENGTH 512
#define USERNAME_LENGTH 32

void sigHandler(int sig);
void * readServer(void* arg);

#endif