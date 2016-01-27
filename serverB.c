/*	Server Side
	There are four servers totally, each using one TCP connected port
	Shall use fork() to handle the four servers as four child processes
*/

#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#include<sys/wait.h>
#include<signal.h>

#define CLIENT_PORT "25141"
#define BACKLOG 5

int info[4][4][2];

/**********************************************************
Function to read adjacency of each server from a text file
**********************************************************/
void get_neighbor_info(int serverNum) {
	/*serverNum indicates which server we are dealing with
	  serverNum = 1 => ServerA
	  serverNum = 2 => ServerB
	  serverNum = 3 => ServerC
	  serverNum = 4 => ServerD
	*/
	FILE *fin;
	char name[10];
	int weight, i, j, k;
	for(i=0; i<4; i++) 
		for(j=0; j<4; j++)
			for(k=0; k<2; k++)
			info[i][j][k] = 0;

	switch(serverNum) {
		case 1: fin = fopen("serverA.txt","r");
			while(!feof(fin)) {
				if(fscanf(fin,"%s%d",name,&weight)!=EOF) {
					if(strcmp(name,"serverB")==0) {
						info[0][1][0] = 2;
						info[0][1][1] = weight;
						}
					if(strcmp(name,"serverC")==0) {
						info[0][2][0] = 3;
						info[0][2][1] = weight;
						}
					if(strcmp(name,"serverD")==0) {
						info[0][3][0] = 4;
						info[0][3][1] = weight;
						}
					}				
				}
			fclose(fin);
			break;
		case 2: fin = fopen("serverB.txt","r");
			printf("The Server B has the following neighbor information\nNeighbor-------Cost\n");
			while(!feof(fin)) {
				if(fscanf(fin,"%s%d",name,&weight)!=EOF) {
					printf("%s\t\t%d\n", name, weight);
					if(strcmp(name,"serverA")==0) {
						info[1][0][0] = 1;
						info[1][0][1] = weight;
						}
					if(strcmp(name,"serverC")==0) {
						info[1][2][0] = 3;
						info[1][2][1] = weight;
						}
					if(strcmp(name,"serverD")==0) {
						info[1][3][0] = 4;
						info[1][3][1] = weight;
						}
					}				
				}
			fclose(fin);
			break;
		case 3: fin = fopen("serverC.txt","r");
			while(!feof(fin)) {
				if(fscanf(fin,"%s%d",name,&weight)!=EOF) {
					if(strcmp(name,"serverA")==0) {
						info[2][0][0] = 1;
						info[2][0][1] = weight;
						}
					if(strcmp(name,"serverB")==0) {
						info[2][1][0] = 2;
						info[2][1][1] = weight;
						}
					if(strcmp(name,"serverD")==0) {
						info[2][3][0] = 4;
						info[2][3][1] = weight;
						}
					}				
				}
			fclose(fin);
			break;
		case 4: fin = fopen("serverD.txt","r");
			while(!feof(fin)) {
				if(fscanf(fin,"%s%d",name,&weight)!=EOF) {
					if(strcmp(name,"serverB")==0) {
						info[3][1][0] = 2;
						info[3][1][1] = weight;
						}
					if(strcmp(name,"serverC")==0) {
						info[3][2][0] = 3;
						info[3][2][1] = weight;
						}
					if(strcmp(name,"serverA")==0) {
						info[3][0][0] = 1;
						info[3][0][1] = weight;
						}
					}				
				}
			fclose(fin);
			break;
		}
	}

/*****************************
Function to get the IP address
*****************************/
void *get_internet_address(struct sockaddr *sa) {
	//Getting IPv4 or IPv6 address
	if(sa->sa_family == AF_INET)	//IPv4
		return &(((struct sockaddr_in*)sa)->sin_addr);
	else				//IPv6
		return &(((struct sockaddr_in6*)sa)->sin6_addr);
	}
/******************
Main function block
******************/
int main(void) {
	printf("Server B is up and running\n\n");
	pid_t pid;
	int i, j, k, l, yes = 1;
	int sockID, newID;	//Socket IDs
	struct addrinfo hints, *serverInfo, *P;
	socklen_t  sin_size;	//socklen_t is of type unsigned integer of atleast 32 bits;
	int rv;
	int serverID;
	char s[INET6_ADDRSTRLEN];
	int buf[8];
	int connectivity[4][2];
	
	serverID = 2;
	get_neighbor_info(serverID);
	printf("\n");
	l = 0;
	for(j=0; j<4; j++) {
		for(k=0; k<2; k++) {
			buf[l++] = info[serverID-1][j][k];
			}
		}
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((rv = getaddrinfo("localhost", CLIENT_PORT, &hints, &serverInfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
		}
	// loop through all the results and connect to the first we can
	for(P = serverInfo; P != NULL; P = P->ai_next) {
		if ((sockID = socket(P->ai_family, P->ai_socktype, P->ai_protocol)) == -1) {
			perror("socket");
			continue;
			}
		if (setsockopt(sockID, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
		}
		if (connect(sockID, P->ai_addr, P->ai_addrlen) == -1) {
			close(sockID);
			perror("connect");
			continue;
			}
		break;
		}

	if (P == NULL) {
		fprintf(stderr, "failed to connect\n");
		return 2;
		}
	//Sending
	write(sockID, buf,sizeof(buf));
	printf("The Server B finishes sending its neighbor information to the client with TCP port number = %s and IP address = %s\n", CLIENT_PORT, inet_ntop(P->ai_family, get_internet_address((struct sockaddr *)P->ai_addr), s, sizeof s));
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	if (getsockname(sockID, (struct sockaddr *)&sin, &len) == -1)
		perror("getsockname"); 
	char hostname[128];
	gethostname(hostname, sizeof hostname);
	printf("For this connection with the client, the Server B has TCP port number = %d and IP address = %s\n",ntohs(sin.sin_port),s);
	freeaddrinfo(serverInfo); // all done with this structure
	close(sockID);			
	printf("\n");
	/*-------------------------------------------------------
	Shall now receive the network map from the client via UDP
	-------------------------------------------------------*/

	int num_bytes;
	socklen_t address_length;
	struct sockaddr_in clientInfo;
	address_length = sizeof(clientInfo);
	char s_a[INET6_ADDRSTRLEN];
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	memset(&serverInfo, 0, sizeof serverInfo);

	if((rv = getaddrinfo(NULL, "22141", &hints, &serverInfo)) != 0) {//if getaddrinfo didn't succeed
		fprintf(stderr,"getaddrinfo : %s\n", gai_strerror(rv));
		return 1;
		}
	//If getaddrinfo succeeded, loop through addresses till we can bind to one
	for(P=serverInfo; P!=NULL; P->ai_next) {
		if((sockID = socket(P->ai_family, P->ai_socktype, P->ai_protocol)) == -1) {
			perror("listener: socket");	
			continue;
			}
		if(bind(sockID, P->ai_addr, P->ai_addrlen) == -1) {
			perror("listener: bind");
			continue;
			}
		break;	//Created and bound to socket
		}
	 if(P==NULL) {
		fprintf(stderr,"Failed to bind");
		return 2;
		}
	int buf_rx[1024];
	freeaddrinfo(serverInfo);
	if((num_bytes = recvfrom(sockID, buf_rx, 1024, 0, (struct sockaddr *)&clientInfo, &address_length)) == -1) {
		perror("recv");
		exit(1);
		}
	socklen_t UDPlen;
	struct sockaddr UDPaddr;
	int UDPport;

	UDPlen = sizeof(UDPaddr);
	getpeername(sockID, (struct sockaddr*)&UDPaddr, &UDPlen);
	struct sockaddr_in *s2 = (struct sockaddr_in *)&UDPaddr;
	UDPport = ntohs(s2->sin_port);

	printf("The server B has received the network topology from the client with UDP port number = %d, and IP address = %s\n",ntohs(clientInfo.sin_port), s);
	printf("Edge------------Cost\n");
	char Vertex[5] = "ABCD";
	int adjacencyMatrix[4][4];
	k = 0;
	for(i=0; i<4; i++)
		for(j=0; j<4; j++)
			adjacencyMatrix[i][j] = buf_rx[k++];

	for(i=0; i<4; i++) {
		for(j=i; j<4; j++) {
			if(adjacencyMatrix[i][j]>0) {
				printf("%c%c\t\t%d\n",Vertex[i], Vertex[j], adjacencyMatrix[i][j]);
				}
			}
		}
	struct sockaddr_in sin3;
	socklen_t len3 = sizeof(sin3);
	if (getsockname(sockID, (struct sockaddr *)&sin3, &len3) == -1)
		perror("getsockname");
	printf("For this connection with the client, the Server B has the UDP port number = %d, and IP address = %s\n",ntohs(sin3.sin_port),s);
	
	close(sockID);
	return 0;
	}
