
#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#define SERVER_PORT 4747
#define MAX_CLIENTS 10
#define BUFFER_LENGTH 512
#define USERNAME_LENGTH 32

void * getMessage(void * arg);
void sendMessage(char* buffer, int length, int caller);
void signalHandler(int signal);

struct Client
{
	pthread_t thread;
	int ID;
	char username[USERNAME_LENGTH];
};

#endif