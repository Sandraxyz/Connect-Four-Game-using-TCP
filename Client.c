#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define RCVBUFSIZE 32   /* Size of receive buffer */

void DieWithError(char *errorMessage);  /* Error handling function */

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
} client2clientSenderMessage;  

int main(int argc, char *argv[])
{
    int sock;                        /* Socket descriptor */
    int servSock;                    /* Socket descriptor for server */
    int clntSock;                    /* Socket descriptor for client */
    int clntSock1;
    int recvMsgSize;	
    struct sockaddr_in servAddr;     /* server address */
    struct sockaddr_in echoClntAddrr;/*sender's client address*/
    struct sockaddr_in ClntAddrr;   /*receiving client address*/
    struct sockaddr_in fromAddr;    /* Source address of server */
    char *servIP;                    /* Server IP address (dotted quad) */
	unsigned int fromSize;           /* In-out of address size for recvfrom() */
	int respStringLen;               /* Length of received response */
    char echoBuffer[RCVBUFSIZE];     /* Buffer for echo string */
    int bytesRcvd, totalBytesRcvd;   /* Bytes read in single recv() and total bytes read */
    int userChc;                     /* Input choice for client to server communication*/
    int userChc1,userChc2;                    /* Input choice for client to client communication*/
    unsigned int clntLen;            /* Length of client address data structure */
	client2ServerMessage *client = malloc(sizeof(client2ServerMessage)); 		//send's data to server		
	serverMessage server;			 // receives data from server
    client2clientSenderMessage clientSenderMessage;
    client2clientSenderMessage clientReceiverMessage;
	int i;                            //loop variable
    int serverPort;					  // serverPort
    int player;                        //Id of the player 
  	
    
    if (argc !=3)    /* Test for correct number of arguments */
    {
       fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n",
               argv[0]);
       exit(1);
    }
    
    servIP = argv[1];             /* First arg: server IP address (dotted quad) */
    serverPort = atoi(argv[2]);    /*Second arg: server port for UDP connection to the server*/
    
	
	//accepting player id
	printf("\nEnter the playerID :- ");
	scanf("%d",&client->playerDD);
	
	
    //accepting port number
	printf("\nEnter the TCP connection port number  :- ");
    scanf("%d",&client->tcpPort);	/* Use given port */

    if(fork () == 0) {

        /* Create socket for incoming connections */
        if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
            DieWithError("socket() failed");

        /* Construct local address structure */
        memset(&ClntAddrr, 0, sizeof(ClntAddrr));   /* Zero out structure */
        ClntAddrr.sin_family = AF_INET;                /* Internet address family */
        ClntAddrr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
        ClntAddrr.sin_port = htons(client->tcpPort);      /* Local port */

        /* Bind to the local address */
        if (bind(servSock, (struct sockaddr *) &ClntAddrr, sizeof(ClntAddrr)) < 0)
            DieWithError("bind() failed");

        /* Mark the socket so it will listen for incoming connections */
        if (listen(servSock, 5) < 0)
            DieWithError("listen() failed");    

        for(;;)
        {
            clntLen = sizeof(echoClntAddrr);
            /* Wait for a client to connect */
            clntSock = accept(servSock, (struct sockaddr *) &echoClntAddrr,&clntLen);

            recvMsgSize = recv(clntSock, &clientSenderMessage, sizeof(&clientSenderMessage), 0);

            printf("\n Received request from %d to play press 1 you want to play, 2 to deny :- ",clientSenderMessage.playerID);
            scanf("%d",userChc2);

            //send accept request
            if(userChc2 == 1){
                  clientReceiverMessage.requestType = acceptc;
                  clientReceiverMessage.playerID =client->playerDD;
            }
            else if(userChc2 == 2)
            {
                clientReceiverMessage.requestType = decline;
                clientReceiverMessage.playerID =client->playerDD;
            }
            else{
                printf("\nInvalid choice");
            }
            printf("Request send successfuly");

            send(clntSock, &clientReceiverMessage, sizeof(&clientReceiverMessage), 0);
            

        }
    }


		
    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");
    
    /* Construct the server address structure */
    memset(&servAddr, 0, sizeof(servAddr));     /* Zero out structure */
    servAddr.sin_family      = AF_INET;             /* Internet address family */
    servAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
    servAddr.sin_port        = htons(serverPort); /* Server port */
        
    /* Send the player information to the server for login*/
	client->requestType=login;
	sendto(sock, client, sizeof(*client), 0, (struct sockaddr *)&servAddr, sizeof(servAddr)) ;

    do{
        printf("\n1 for who, 2 for lookup, 3 for logout : ");
        scanf("%d",&userChc);

        if(userChc == 1) {
	        /*Retrieving the list of players */
            
            client->requestType = who;
            sendto(sock, client, sizeof(*client), 0, (struct sockaddr *)&servAddr, sizeof(servAddr)) ;
	        fromSize = sizeof(fromAddr);
	        respStringLen = recvfrom(sock, &server, sizeof(server), 0,(struct sockaddr *) &fromAddr, &fromSize);
	        printf("\n logged in players are :-");
	        for(i=0;i<10;i++){
                if(server.availPlayers[i] <= 9999 ){
		            printf("%d ",server.availPlayers[i]);
                }
                else
                    break ;    
	        }
            printf("\n");
        }
        else if(userChc == 2){
            printf("\n Enter the id of the player : ");
            scanf("%d",&client->playerDD);
            client->requestType =lookup;
            sendto(sock, client, sizeof(*client), 0, (struct sockaddr *)&servAddr, sizeof(servAddr));
            fromSize = sizeof(fromAddr);
	        respStringLen = recvfrom(sock, &server, sizeof(server), 0,(struct sockaddr *) &fromAddr, &fromSize);
           printf("\n Player %d IP Adress is %s  and TCP port is %d",server.requestedID, server.ipAddress,server.tcpPort);
           printf("\n Enter 1 to send request, 2 to not send request :- ");
           scanf("%d",&userChc1);

           if(userChc1 == 1){
               /* Create a reliable, stream socket using TCP */
                if ((clntSock1 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
                    DieWithError("socket() failed");

                /*constructing communication with the client*/
                memset(&echoClntAddrr, 0, sizeof(echoClntAddrr));     /* Zero out structure */
                echoClntAddrr.sin_family      = AF_INET;             /* Internet address family */
                echoClntAddrr.sin_addr.s_addr = inet_addr(server.ipAddress);   /* Server IP address */
                echoClntAddrr.sin_port        = htons(server.tcpPort);

                /* Establish the connection to the echo server */
                if (connect(clntSock1, (struct sockaddr *) &echoClntAddrr, sizeof(echoClntAddrr)) < 0)
                    DieWithError("connect() failed");

                clientSenderMessage.move=0;
                clientSenderMessage.playerID=client->playerDD;
                clientSenderMessage.requestType=play;

                /*sending play request to the another player*/
                printf("\nSend the request to player %d Waiting for Approval",clientSenderMessage.playerID);
                send(clntSock1, &clientSenderMessage, sizeof(clientSenderMessage), 0);

                recvMsgSize = recv(clntSock1, &clientSenderMessage, sizeof(&clientSenderMessage), 0);

                printf("\nReceived request %d",clientSenderMessage.requestType);

    
           } 
        }
        else if(userChc == 3) {
            client->requestType = logout;
            sendto(sock, client, sizeof(*client), 0, (struct sockaddr *)&servAddr, sizeof(servAddr));
            exit(0);
        }
        else{
            printf("\nInvalid choice entered");
        }
    }while(1);

   
    
    close(sock);
    exit(0);
}
