#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

int clientCount = 0;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

struct client{

	int index;
	int sockID;
	int groupID[100];
	int groupNumber;
	struct sockaddr_in clientAddr;
	char *username;
	int len;

};


struct client Client[1024];
pthread_t thread[1024];

void * doNetworking(void * ClientDetail){

	struct client* clientDetail = (struct client*) ClientDetail;
	int index = clientDetail -> index;
	int clientSocket = clientDetail -> sockID;
	Client[index].groupNumber = 0;
	char username[1024];
	int end = recv(clientSocket,username,1024,0);
	username[end] = '\0';
	Client[index].username = username;
	char  sendingString[1024] ;

	printf("Client %s connected.\n",Client[index].username);

	while(1){

		char data[1024];
		int read = recv(clientSocket,data,1024,0);
		data[read] = '\0';

		char output[1024];
		
		if(strcmp(data,"join") == 0){
			printf("join requested by Client %s\n" , Client[index].username);
			read = recv(clientSocket,data,1024,0);
			data[read] = '\0';
			int id = atoi(data);
			int exist = 0;
			for(int i = 0 ; i< Client[index].groupNumber ; i++){
				if(Client[index].groupID[i] == id){
					exist = 1;
				}
			}
			if(exist == 0){
				Client[index].groupID[Client[index].groupNumber] = id;
				Client[index].groupNumber++;
			}

			continue;
		}
		if(strcmp(data,"send") == 0){

			read = recv(clientSocket,data,1024,0);
			data[read] = '\0';

			int id = atoi(data);

			
			read = recv(clientSocket,data,1024,0);
			data[read] = '\0';

			int isMember = 0;
			
			for(int i = 0 ; i < Client[index].groupNumber ; i++){
				if(Client[index].groupID[i] == id){
					isMember = 1;
				}
			}
			if(isMember == 0) continue;
			strcpy(sendingString , Client[index].username);
			strcat(sendingString , ": ");
			strcat(sendingString , data);
			for(int i = 0 ; i < clientCount ; i++){
				for(int j = 0 ; j < Client[i].groupNumber ; j++){
					if(Client[i].groupID[j] == id){
						send(Client[i].sockID,sendingString,1024,0);	
					}
				}
			}
			memset(sendingString , 0 , strlen(sendingString));

			continue;

		}
		if(strcmp(data,"leave") == 0){
			read = recv(clientSocket,data,1024,0);
			data[read] = '\0';

			int id = atoi(data);

			for(int i = 0 ; i < Client[index].groupNumber ; i++){
				if(Client[index].groupID[i] == id){
					Client[index].groupID[i] = -1;
				}
			}
		}

	}

	return NULL;

}

int main(int argc, char *argv[]){

	int serverSocket = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serverAddr;


	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(argv[1]));
	serverAddr.sin_addr.s_addr = htons(INADDR_ANY);


	if(bind(serverSocket,(struct sockaddr *) &serverAddr , sizeof(serverAddr)) == -1) return 0;

	if(listen(serverSocket,1024) == -1) return 0;

	printf("Server started listenting on port %s ...........\n" , argv[1]);

	while(1){

		Client[clientCount].sockID = accept(serverSocket, (struct sockaddr*) &Client[clientCount].clientAddr, &Client[clientCount].len);
		Client[clientCount].index = clientCount;

		pthread_create(&thread[clientCount], NULL, doNetworking, (void *) &Client[clientCount]);

		clientCount ++;
 
	}


}
