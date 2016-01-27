/*	Client Side
	There is only one client
	Client will receive connectivity information of each server via TCP
	and will accordingly build a graph. This connected graph will be 
	sent to each server via UDP
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#include<signal.h>
#include<sys/wait.h>

#define CLIENT_PORT "25141"
#define BACKLOG 5
#define INT_MAX 10000
typedef enum {false, true} bool;

/*--------------------------
get sockaddr, IPv4 or IPv6:
--------------------------*/
void *get_internet_address(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET)
		return &(((struct sockaddr_in*)sa)->sin_addr);
	else
		return &(((struct sockaddr_in6*)sa)->sin6_addr);
	}

/*-----------------------------------------------------------------------------------------
MST_Prim is a function that computes the minimum spanning tree for a given adjacency matrix
-----------------------------------------------------------------------------------------*/
int minimumKey(int key[], bool mstSet[]) {	//This utility function was referenced from geeks4geeks.com
	int min, min_index, i; 
	min  = INT_MAX;
	for (i = 0; i < 4; i++)
	if (mstSet[i] == false && key[i] < min)
		min = key[i], min_index = i;
	return min_index;
	}

void MST_Prim(int graph[4][4]) {
	int parent[4];		//Array to store constructed MST
	int key[4], i, count, u, v;	//Used to pick minimum weight edge during a particular iteration
	bool mstSet[4];		//Vertices not yet included in MST
 
     	//Initialize all keys as INFINITE so that we can fid min weight edge by comparison
	for(i = 0; i < 4; i++)
		key[i] = INT_MAX, mstSet[i] = false;

     	//Always include first 1st vertex in MST.
     	key[0] = 0;     	//Selecting first vertex, which is always serverA
     	parent[0] = -1; 	//First node is always root of MST 
 
     	//The MST will have 4 vertices, since only 4 servers are present
     	for (count = 0; count < 3; count++) {
	        //Pick thd minimum key vertex from the set of vertices not yet included in MST
        	u = minimumKey(key, mstSet);
 	        // Add the picked vertex to the MST Set
 	        mstSet[u] = true;
	        /* Update key value and parent index of the adjacent vertices of the picked vertex. 
		Consider only those vertices which are not yet included in MST*/
        	for (v = 0; v < 4; v++)
        		if (graph[u][v] && mstSet[v] == false && graph[u][v] <  key[v])
        			parent[v]  = u, key[v] = graph[u][v];
     		}
	char vertex[5] = "ABCD";
	printf("Edge-----------Weight\n");
	for (i = 1; i < 4; i++)
		printf("%c%c\t\t%d\n", vertex[parent[i]], vertex[i], graph[i][parent[i]]);
}
/*-----------------
Main Function Body
-----------------*/
int main(void) {
	printf("The Client is up and running\n");
	int sockID, newID, rv, yes = 1, i, j, k, l;
	char s[INET6_ADDRSTRLEN];
	struct addrinfo hints, *serverInfo, *P;
	struct sockaddr_in clientInfo;
	int netInformation[4][4][2], rxBuf[8], num_bytes;
	socklen_t sin_size;	//socklen_t is of type unsigned integer of at least 32 bits

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;	//Unsure if IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;//TCP socket
	hints.ai_flags = AI_PASSIVE;	//Use local IP address
	struct sigaction sa;
	//getaddrsinfo returns one or more addrinfo structures each of which contains a valid internet address
	//It returns 0 if it succeeds
	if((rv = getaddrinfo("localhost", CLIENT_PORT, &hints, &serverInfo))!=0) {	//If getaddrinfo did not succeed
		fprintf(stderr, "getaddrsinfo: %s\n", gai_strerror(rv));
		return 1;
		}
	//Looping through all getaddrsinfo results until we bind to one
	for(P=serverInfo; P!=NULL; P=P->ai_next) {
		if((sockID = socket(P->ai_family, P->ai_socktype, P->ai_protocol)) == -1) {
			perror("server: socket");	//Failed to create socket
			continue;
			}
		//setsockopt() sets the option specified by the third argument
		//at the protocol level specified by the second argument
		if(setsockopt(sockID, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
			}
		if(bind(sockID, P->ai_addr, P->ai_addrlen) == -1) {
			close(sockID);
			perror("server: bind");		//Failed to bind to P-th address
			continue;
			}
		break;
		}
	freeaddrinfo(serverInfo);	//Clear up the structure
	if(P == NULL) {
		fprintf(stderr,"Server: failed to bind");	//Could not bind with any of the addresses
		exit(1);
		}
	if(listen(sockID,BACKLOG) == -1) {
		perror("Listen");
		exit(1);
		}

	struct sockaddr_in sin0;
	socklen_t len0 = sizeof(sin0);
	if (getsockname(sockID, (struct sockaddr *)&sin0, &len0) == -1)
		perror("getsockname");
	
	printf("It has TCP port number = %d and IP address = %s\n",ntohs(sin0.sin_port), inet_ntoa(sin0.sin_addr));
	sin_size = sizeof clientInfo;
	i = 0;
	char Vertex[5] = "ABCD";
	socklen_t TCPlen;
	struct sockaddr TCPaddr;
	int TCPport;
	char ipstr[INET6_ADDRSTRLEN];
	TCPlen = sizeof(TCPaddr);
	for(i=0; i<4; i++) {
		printf("\n");
		newID = accept(sockID, (struct sockaddr *)&clientInfo, &sin_size);
		if(newID == -1) {
			perror("accept");
		 	//continue;
			}
		//Reading from TCP socket	
		read(newID, rxBuf, sizeof(rxBuf));
		printf("The client receives the neighbor information from server%c with TCP port number = %d, and IP address = %s\n",Vertex[i], ntohs(clientInfo.sin_port), inet_ntoa(clientInfo.sin_addr) );
		printf("Server%c has the following neighbor information :\nNeighbor-------Cost\n",Vertex[i]);
		l = 0;	
		for(j=0; j<4; j++)
			for(k=0; k<2; k++)
				netInformation[i][j][k] = rxBuf[l++];
		for(j=0; j<4; j++) {
				if(netInformation[i][j][0]>0)
					printf("Server%c\t\t%d\n",Vertex[j],netInformation[i][j][1]);
				}
		close(newID);
		printf("For this connection with Server%c, the client has a TCP port number = %d, and IP address = %s\n",Vertex[i],ntohs(sin0.sin_port), inet_ntoa(sin0.sin_addr));
		}
	int adjacencyMatrix[4][4], networkMap[16];
	for(i=0; i<4; i++)
		for(j=0; j<4; j++)
			adjacencyMatrix[i][j] = 0;
	for(i=0; i<4; i++) {
		for(j=0; j<4; j++) {
			if(netInformation[i][j][0] != 0) 
				adjacencyMatrix[i][j] = netInformation[i][j][1];
			}
		}
	l = 0;
	for(i=0; i<4; i++) {
		for(j=0; j<4; j++) {
			networkMap[l++] = adjacencyMatrix[i][j];
			}
		}
	//Got the connectivity information from each server and obtained the adjacency matrix
	/*---------------------------------------------------------------------------------------
	Now sending this network map denoted by the adjacency matrix to a node in the network
	---------------------------------------------------------------------------------------*/
//Sending to Server A	
	printf("\n");
	memset(&hints,0,sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	if((rv = getaddrinfo("localhost", "21141", &hints, &serverInfo)) != 0) {//getaddrinfo didn't succeed
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
		}
	//Looping through all getaddrinfo results to make a socket
	for(P=serverInfo; P!=NULL; P = P->ai_next) {
		if((sockID = socket(P->ai_family, P->ai_socktype, P->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
			}
		break;
		}
	if(P == NULL) {
		fprintf(stderr, "talker: Unable to create socket");
		exit(1);
		}
	if((num_bytes = sendto(sockID, networkMap, 1024, 0, P->ai_addr, P->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
		}
	struct sockaddr_in sin3;
	socklen_t len3 = sizeof(sin3);
	if (getsockname(sockID, (struct sockaddr *)&sin3, &len3) == -1)
		perror("getsockname"); 
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	char ip4[INET_ADDRSTRLEN];
	getpeername(sockID, (struct sockaddr*)&sin3, &len3);
	inet_ntop(AF_INET, &(sin3.sin_addr), ip4, INET_ADDRSTRLEN);
	if (getsockname(sockID, (struct sockaddr *)&sin0, &len0) == -1)
		perror("getsockname");
	printf("The Client has sent the network topology to ServerA with UDP port = %d, and IP address = %s as follows : \n", 21141,inet_ntop(P->ai_family, get_internet_address((struct sockaddr *)P->ai_addr), s, sizeof s) );
	printf("Edge------------Cost\n");
	for(i=0; i<4; i++) {
		for(j=i; j<4; j++) {
			if(adjacencyMatrix[i][j]>0) {
				printf("%c%c\t\t%d\n",Vertex[i], Vertex[j], adjacencyMatrix[i][j]);
				}
			}
		}
	printf("For this connection with ServerA, the client has UDP port = %d, and IP address = %s\n", ntohs(sin0.sin_port),inet_ntop(P->ai_family, get_internet_address((struct sockaddr *)P->ai_addr), s, sizeof s));
	close(sockID);
	freeaddrinfo(serverInfo);
//Sending to Server B
	printf("\n");
	memset(&hints,0,sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	if((rv = getaddrinfo("localhost", "22141", &hints, &serverInfo)) != 0) {//getaddrinfo didn't succeed
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
		}
	//Looping through all getaddrinfo results it make a socket
	for(P=serverInfo; P!=NULL; P = P->ai_next) {
		if((sockID = socket(P->ai_family, P->ai_socktype, P->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
			}
		break;
		}
	if(P == NULL) {
		fprintf(stderr, "talker: Unable to create socket");
		exit(1);
		}
	if((num_bytes = sendto(sockID, networkMap, 1024, 0, P->ai_addr, P->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
		}
	if (getsockname(sockID, (struct sockaddr *)&sin3, &len3) == -1)
		perror("getsockname");
	printf("The Client has sent the network topology to ServerB with UDP port = %d, and IP address = %s as follows : \n", 22141, inet_ntop(P->ai_family, get_internet_address((struct sockaddr *)P->ai_addr), s, sizeof s));
	printf("Edge------------Cost\n");
	for(i=0; i<4; i++) {
		for(j=i; j<4; j++) {
			if(adjacencyMatrix[i][j]>0) {
				printf("%c%c\t\t%d\n",Vertex[i], Vertex[j], adjacencyMatrix[i][j]);
				}
			}
		}
	printf("For this connection with ServerB, the client has UDP port = %d, and IP address = %s\n", htons(sin3.sin_port), inet_ntop(P->ai_family, get_internet_address((struct sockaddr *)P->ai_addr), s, sizeof s));
	close(sockID);
	freeaddrinfo(serverInfo);
//Sending to Server C	
	printf("\n");
	memset(&hints,0,sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	if((rv = getaddrinfo("localhost", "23141", &hints, &serverInfo)) != 0) {//getaddrinfo didn't succeed
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
		}
	//Looping through all getaddrinfo results it make a socket
	for(P=serverInfo; P!=NULL; P = P->ai_next) {
		if((sockID = socket(P->ai_family, P->ai_socktype, P->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
			}
		break;
		}
	if(P == NULL) {
		fprintf(stderr, "talker: Unable to create socket");
		exit(1);
		}
	if((num_bytes = sendto(sockID, networkMap, 1024, 0, P->ai_addr, P->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
		}
	if (getsockname(sockID, (struct sockaddr *)&sin3, &len3) == -1)
		perror("getsockname"); 
	printf("The Client has sent the network topology to ServerC with UDP port = %d, and IP address = %s as follows : \n", 23141, inet_ntop(P->ai_family, get_internet_address((struct sockaddr *)P->ai_addr), s, sizeof s));
	printf("Edge------------Cost\n");
	for(i=0; i<4; i++) {
		for(j=i; j<4; j++) {
			if(adjacencyMatrix[i][j]>0) {
				printf("%c%c\t\t%d\n",Vertex[i], Vertex[j], adjacencyMatrix[i][j]);
				}
			}
		}
	printf("For this connection with ServerC, the client has UDP port = %d, and IP address = %s\n", htons(sin3.sin_port), inet_ntop(P->ai_family, get_internet_address((struct sockaddr *)P->ai_addr), s, sizeof s));
	close(sockID);
	freeaddrinfo(serverInfo);
//Sending to Server D
	printf("\n");
	memset(&hints,0,sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	if((rv = getaddrinfo("localhost", "24141", &hints, &serverInfo)) != 0) {//getaddrinfo didn't succeed
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
		}
	//Looping through all getaddrinfo results it make a socket
	for(P=serverInfo; P!=NULL; P = P->ai_next) {
		if((sockID = socket(P->ai_family, P->ai_socktype, P->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
			}
		break;
		}
	if(P == NULL) {
		fprintf(stderr, "talker: Unable to create socket");
		exit(1);
		}
	if((num_bytes = sendto(sockID, networkMap, 1024, 0, P->ai_addr, P->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
		}
	if (getsockname(sockID, (struct sockaddr *)&sin3, &len3) == -1)
		perror("getsockname"); 
	printf("The Client has sent the network topology to ServerD with UDP port = %d, and IP address = %s as follows : \n", 24141, inet_ntop(P->ai_family, get_internet_address((struct sockaddr *)P->ai_addr), s, sizeof s));
	printf("Edge------------Cost\n");
	for(i=0; i<4; i++) {
		for(j=i; j<4; j++) {
			if(adjacencyMatrix[i][j]>0) {
				printf("%c%c\t\t%d\n",Vertex[i], Vertex[j], adjacencyMatrix[i][j]);
				}
			}
		}
	printf("For this connection with ServerD, the client has UDP port = %d, and IP address = %s\n", htons(sin3.sin_port), inet_ntop(P->ai_family, get_internet_address((struct sockaddr *)P->ai_addr), s, sizeof s));
	close(sockID);
	freeaddrinfo(serverInfo);
	printf("\n");
//*************************************************************************
//We shall now find the minimum spanning tree
//*************************************************************************
	printf("We have now finished sending the network map to each of the servers\nThe Client has calculated the minimum spanning tree so that we can find out the optimal routing path as follows : \n");
	MST_Prim(adjacencyMatrix);	
	return 0;
	}
	
