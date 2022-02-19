#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define MAXPENDING 5    /* Maximum outstanding connection requests */
#define RCVBUFSIZE 32   /* Size of receive buffer */

typedef struct {
	enum {login, who, lookup, logout } requestType;     /* same size as an unsigned int */
	unsigned int playerDD;                       /* initiating player identifier */
	unsigned short tcpPort;                     /* listening port*/
} client2ServerMessage;                      

typedef struct {
	enum {ok, lookups, whos} responseType;        /* same size as an unsigned int */
	int availPlayers[10];                         /* list of player identifiers */
	unsigned int requestedID;                 /* requested client id */
	unsigned short tcpPort;                     /* requested port */
	char ipAddress[30];                     /* requested IP address */
} serverMessage;    

typedef struct {
	enum {play, acceptc, decline, move} requestType;    /* same size as an unsigned int */
	unsigned int playerID;                 /* initiating player identifier */
	unsigned int move;                     /* destination column – 1, 2, 3, 4, 5, 6, or 7 */
} client2ClientMessage;  
void DieWithError(char *errorMessage);  /* Error handling function */

void HandleTCPClient(int clntSocket)
{
    char echoBuffer[RCVBUFSIZE];        /* Buffer for echo string */
    int recvMsgSize;                    /* Size of received message */
    
    /* Receive message from client */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");
    
    /* Send received string and receive again until end of transmission */
    while (recvMsgSize > 0)      /* zero indicates end of transmission */
    {
        /* Echo message back to client */
        if (send(clntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
            DieWithError("send() failed");
        
        /* See if there is more data to receive */
        if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
            DieWithError("recv() failed");
    }
    
    close(clntSocket);    /* Close client socket */
}

int main(int argc, char *argv[])
{
    int servSock;                    /* Socket descriptor for server */
    int clntSock;                    /* Socket descriptor for client */
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned short echoServPort;     /* Server port */
    unsigned int clntLen;            /* Length of client address data structure */
	client2ServerMessage client; 	// send's data to client	
	int recvMsgSize;
    int availablePlayersCount=0;
	serverMessage server[10];			 // receives data from client
    serverMessage selectedPlayer;       //particular player whose information is needed to send
    int i=0,selectedPlayerIndex=0;
    
    if (argc != 2)     /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Server Port>\n", argv[0]);
        exit(1);
    }
    
    echoServPort = atoi(argv[1]);  /* First arg:  local port */
    
    /* Create socket for incoming UDP connections */
    if ((servSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");
    
    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Local port */
    
    /* Bind to the local address */
    if (bind(servSock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");
    
    /* Mark the socket so it will listen for incoming connections */
    // if (listen(servSock, MAXPENDING) < 0)
    //     DieWithError("listen() failed");
    
    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        clntLen = sizeof(echoClntAddr);
		
		recvMsgSize = recvfrom(servSock, &client, sizeof(client), 0,(struct sockaddr *) &echoClntAddr, &clntLen);

           // DieWithError("recvfrom() failed");


        //login operations	
		if(client.requestType==login){
            //server[availablePlayersCount] = malloc(sizeof(serverMessage));
            server[availablePlayersCount].requestedID = client.playerDD;
            server[availablePlayersCount].tcpPort=client.tcpPort;
            strcpy(server[availablePlayersCount].ipAddress ,inet_ntoa(echoClntAddr.sin_addr));
            
            if(availablePlayersCount ==0 ) {
                server[availablePlayersCount].availPlayers[availablePlayersCount] = client.playerDD;
                availablePlayersCount++;
            }
            else{
                //assigning all the available players to the new players
                for(i = 0; i<availablePlayersCount;i++){
                    server[availablePlayersCount].availPlayers[i] = server[0].availPlayers[i];        
                }
                //assigning new player to available player's list
                for(i = 0;i<=availablePlayersCount;i++){
                     server[i].availPlayers[availablePlayersCount] = client.playerDD;
                }
                availablePlayersCount++;
            }   
        }

        //who operation
        if(client.requestType==who){
            server[availablePlayersCount - 1].responseType=whos;
            sendto(servSock, server, sizeof(*server), 0,(struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr));
        }	

        //Lookup operation
        if(client.requestType == lookup){
            for(i=0;i<=availablePlayersCount;i++){
                if(client.playerDD == server[i].requestedID)
                {
                    selectedPlayer = server[i];
                    sendto(servSock, &selectedPlayer, sizeof(selectedPlayer), 0,(struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr));
                }    
            }
            
        }

        if(client.requestType == logout){
             for(i=0;i<=availablePlayersCount;i++){
                if(client.playerDD == server[i].requestedID)
                {
                    selectedPlayerIndex = i;        
                }    
            }
             for(i=0;i<=availablePlayersCount;i++){
                   server[i].availPlayers[selectedPlayerIndex] = 0;
            }
            
            
        }
		
    }
    /* NOT REACHED */
}
