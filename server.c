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
int groupsNum = 0;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

struct client{

	int index;
	int sockID;
	int groupID;
	struct sockaddr_in clientAddr;
	int len;

};

struct group{
	int ID;
	int memberCount;
	int member[100];
};

struct group Groups[100];
struct client Client[1024];
pthread_t thread[1024];

void * doNetworking(void * ClientDetail){

	struct client* clientDetail = (struct client*) ClientDetail;
	int index = clientDetail -> index;
	int clientSocket = clientDetail -> sockID;

	printf("Client %d connected.\n",index + 1);

	while(1){

		char data[1024];
		int read = recv(clientSocket,data,1024,0);
		data[read] = '\0';

		char output[1024];
		
		if(strcmp(data,"join") == 0){
			printf("join requested by Client %d\n" , index + 1);
			read = recv(clientSocket,data,1024,0);
			data[read] = '\0';
			int id = atoi(data);
			int exist = 0;
			for(int i = 0 ; i< groupsNum ; i++){
				if(Groups[i].ID == id){
					int memberExists = 0;
					for(int j = 0 ; j < Groups[i].memberCount ; j++){
						if(Groups[i].member[j] == clientSocket){
							memberExists = 1;
							exist = 1;
							break;
						}
					}
					if(memberExists == 0){
						Groups[i].member[Groups[i].memberCount] = clientSocket;
						Groups[i].memberCount++;
						exist = 1;
						break;
					}
				}
			}
			if(exist == 0){
				Groups[groupsNum].ID = id;
				Groups[groupsNum].member[0] = clientSocket;
				Groups[groupsNum].memberCount = 1;
				groupsNum++;
			}

			continue;
		}
		if(strcmp(data,"LIST") == 0){

			int l = 0;

			for(int i = 0 ; i < clientCount ; i ++){

				if(i != index)
					l += snprintf(output + l,1024,"Client %d is at socket %d.\n",i + 1,Client[i].sockID);

			}

			send(clientSocket,output,1024,0);
			continue;

		}
		if(strcmp(data,"SEND") == 0){

			read = recv(clientSocket,data,1024,0);
			data[read] = '\0';

			int id = atoi(data) - 1;

			read = recv(clientSocket,data,1024,0);
			data[read] = '\0';

			send(Client[id].sockID,data,1024,0);			

		}

	}

	return NULL;

}

int main(){

	int serverSocket = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serverAddr;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8080);
	serverAddr.sin_addr.s_addr = htons(INADDR_ANY);


	if(bind(serverSocket,(struct sockaddr *) &serverAddr , sizeof(serverAddr)) == -1) return 0;

	if(listen(serverSocket,1024) == -1) return 0;

	printf("Server started listenting on port 8080 ...........\n");

	while(1){

		Client[clientCount].sockID = accept(serverSocket, (struct sockaddr*) &Client[clientCount].clientAddr, &Client[clientCount].len);
		Client[clientCount].index = clientCount;

		pthread_create(&thread[clientCount], NULL, doNetworking, (void *) &Client[clientCount]);

		clientCount ++;
 
	}

	for(int i = 0 ; i < clientCount ; i ++)
		pthread_join(thread[i],NULL);

}
