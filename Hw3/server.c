
#include "server.h"

pthread_mutex_t mutexLock;

int clientCount = 0;
struct Client clients[MAX_CLIENTS];

int main()
{
	int sckt;
	struct sockaddr_in serverAddr = {AF_INET, htons(SERVER_PORT)};
	struct sockaddr_in clientAddr = {AF_INET};
	socklen_t clientLen = sizeof(clientAddr);
	int newClient;
	int i = 0;
  
	//Signal Handler
	signal(SIGINT, signalHandler);

	for(i = 0; i < MAX_CLIENTS; i++)
	{
		clients[i].ID = 0;
	}
  
	sckt = socket(AF_INET, SOCK_STREAM, 0);
	if(sckt == -1)
	{
		perror("Server: Error: Socket set up failed");
		exit(1);
	}
  
	if(bind(sckt, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) == -1)
	{
		perror("Server: Error: Socket bind failed");
		exit(1);
	}

	if(listen(sckt, 1) == -1)
	{
		perror("Server: Error: Listening for Clients failed");
		exit(1);
	}
	
	//Socket set up and listening for clients
	printf("Server: Server is listening for clients\n");

	//Check number of current clients
	while((newClient = accept(sckt, (struct sockaddr*) &clientAddr, &clientLen)) > 0)
	{
		if(clientCount < MAX_CLIENTS)
		{
			pthread_mutex_lock(&mutexLock);
		}
		i = 0;
		while( i < MAX_CLIENTS && clients[i].ID != 0)
		{
			i++;
        }
		clients[i].ID = newClient;
		clientCount++;
		pthread_create(&clients[i].thread, NULL, &getMessage, &clients[i]);
		pthread_mutex_unlock(&mutexLock);
    }

	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if(clients[i].ID > 0)
		{
		pthread_join(clients[i].thread, NULL);
		close(clients[i].ID);
		}
	}

	close(socket);
	unlink(serverAddr.sin_addr);
	
	return 0;
}

//Reads messages from Client
void * getMessage(void* arg)
{
	struct Client *client = arg;
	char buffer[BUFFER_LENGTH];
	char returnMessage[BUFFER_LENGTH+ USERNAME_LENGTH + 3];
	char welcome[ USERNAME_LENGTH + 20];

	//Get username
	read(client->ID, client->username,  USERNAME_LENGTH);
	pthread_mutex_lock(&mutexLock);
	strcat(welcome, client->username);
	strcat(welcome, " has connected\n");
	printf("%s", welcome);
	sendMessage(welcome, strlen(welcome), client->ID);
	pthread_mutex_unlock(&mutexLock);
  
	//Get messages
	while( (read(client->ID, buffer, BUFFER_LENGTH)) != 0)
	{
		pthread_mutex_lock(&mutexLock);
		sprintf(returnMessage, "%s: %s", client->username, buffer);
		printf("%s", returnMessage);
		sendMessage(returnMessage, strlen(returnMessage), client->ID);
		pthread_mutex_unlock(&mutexLock);
	}
  
	//If client leaves message is sent
	pthread_mutex_lock(&mutexLock);
	sprintf(returnMessage, "%s %s", client->username, "has disconnected.\n");
	printf("%s", returnMessage);
	sendMessage(returnMessage, strlen(returnMessage), client->ID);
	close(client->ID);
	client->ID = 0;
	pthread_mutex_unlock(&mutexLock);
	pthread_exit(NULL);
	return;
}

//Sends message to clients
void sendMessage(char* buffer, int length, int caller)
{
	int i = 0;
	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if(clients[i].ID != 0 && clients[i].ID != caller)
		{
		write(clients[i].ID, buffer, length+1);
		}
	}
	return;
}


//Handles user Ctrl+C
void signalHandler(int signal)
{
	if(signal == SIGINT)
	{
		pthread_mutex_lock(&mutexLock);
		printf("\nServer will shutdown in 10 Seconds\n");
		sendMessage("/exit", 6, 0);
		int i;
		for(i = 0; i < MAX_CLIENTS; i++)
		{
			if(clients[i].ID > 0)
			{
			close(clients[i].ID);
			}
		}
		
		close(socket);
		pthread_mutex_unlock(&mutexLock);
		sleep(10);
		exit(1);
	}
}