
#include "client.h"

int quit = 0;

int main(int argc, char* argv[])
{
	struct sockaddr_in serverAddr = {AF_INET, htons(SERVER_PORT)};
	char buffer[BUFFER_LENGTH];
	char username[USERNAME_LENGTH];
    struct hostent* host;
	int sckt;
	pthread_t thread;
	
	//Signal Handler
	signal(SIGINT, sigHandler);
	
	//Setting up socket
    if(argc != 2) 
    { 
		printf("Usage: %s <hostname>\n", argv[0]); 
		exit(1); 
    } 
	
	//Getting host
	host = gethostbyname(argv[1]);
	
	if(host == NULL)
	{
		printf("Unknown Host: %s\n", argv[1]);
		exit(1);
	}
	
	bcopy(host->h_addr_list[0], (char*)&serverAddr.sin_addr, host->h_length);
	
	//Create socket
	sckt = socket(AF_INET, SOCK_STREAM, 0);
	if(sckt == -1) 
    { 
		perror("Error: Socket set up failed"); 
		exit( 1 ); 
    } 
	
	//Connect socket
	if(connect(sckt,(struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) 
    { 
		perror("Error: Connection failed"); 
		exit( 1 ); 
    }
	
	//Connection sucessful
	printf("Connected to %s\n", argv[1]);
	printf("Username: ");
	
	do
	{
		fgets(username, USERNAME_LENGTH, stdin);
	}while(username[0] == '\n');
	
	pthread_create(&thread, NULL, &readServer, &sckt);
	strtok(username, "\n");
	write(sckt, username, USERNAME_LENGTH);
	printf("You are now in the Chat Room as %s!\n", username);
	
	
	while(quit == 0 && fgets(&buffer, BUFFER_LENGTH, stdin) != EOF)
	{
		if(strcmp(buffer, "/exit\n") == 0 || strcmp(buffer, "/quit\n") == 0 || strcmp(buffer, "/part\n") == 0)
		{
			quit = 1;
		}
		else if(strcmp(buffer, "\n") != 0)
		{
			write(sckt, buffer, strlen(buffer) + 1);
		}
		printf("\n");
	}
	
	close(sckt);
	
	return 0;
}

//Signal Handler
void sigHandler(int sig)
{
	if(sig == SIGINT)
	{
		printf("\nTo exit this program please Use /exit, /quit, or /part\n");
	}
}

//Handles messages from the server
void * readServer(void* arg)
{
	int* sckt = arg;
	char message[BUFFER_LENGTH + USERNAME_LENGTH];
	
	while(quit == 0 && read(*sckt, message, BUFFER_LENGTH + USERNAME_LENGTH) != 0)
	{
		if(strcmp(message, "/exit") == 0)
		{
			printf("Server is shutting down.\n");
			printf("Goodbye\n");
			quit = 1;
			sleep(3);
			close(*sckt);
			exit(1);
		}
		else
		{
			printf("%s", message);
		}
	}
	pthread_exit(NULL);
	return 0;
}