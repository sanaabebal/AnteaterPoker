/* clientGUIW9.cpp - BETA */

/* GTK_ClockClient.c: GUI example interactive TCP/IP client for ClockServer
 * Author: Rainer Doemer, 04/22/22 (based on simple ClockClient.c)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
// #include <gtk/gtk.h>
#include <assert.h>

#include "cards.hpp"
#include "data.hpp"
#include "DataTransfer.hpp"


#define DEFAULT_SERVERPORT 10080
#define DEFAULT_SERVERNAME "bondi"


/* #define DEBUG */	/* be verbose */


/*** global variables ****************************************************/

const char *Program	/* program name for descriptive diagnostics */
	= NULL;
struct sockaddr_in
	ServerAddress;	/* server address we connect with */
int SocketFD = 0; // used by several programs

GAMESTATE officialGameState;
LOGININFO playerLoginInfo;


/*** global functions ****************************************************/

void FatalError(		/* print error diagnostics and abort */
	const char *ErrorMsg)
{
    fputs(Program, stderr);
    fputs(": ", stderr);
    perror(ErrorMsg);
    fputs(Program, stderr);
    fputs(": Exiting!\n", stderr);
    exit(20);
} /* end of FatalError */

LOGININFO Talk2ServerLogin(LOGININFO loginInfo){	/* communicate with the server */
    int n;

    /* Creating buffer, sending it to server */
    BUF RecvBuf(2000); /* message buffer for receiving a message -- think the max for login and game is well below 2000 bytes, but using this just in case */
    BUF SendBuf; /* message buffer for sending a response */

    SendBuf = createBuffer(loginInfo);
    n = write(SocketFD, SendBuf.data(), SendBuf.size());
    if (n < 0){   
        FatalError("writing to socket failed");
    }
    
    /* Getting response from server and closing the socket */
    n = read(SocketFD, RecvBuf.data(), 2000);
    if (n < 0) {   
        FatalError("reading from socket failed");
    }
    RecvBuf.resize(n);

    // Wrapup:  Getting new structure and returning it
    loginInfo = parsingLoginArguments(RecvBuf);
    return loginInfo;
} /* end of Talk2Server */

LOGININFO SendServerLogin(LOGININFO loginInfo){ // full process of sending LOGININFO structure to server and receiving confirmation; debugging printing included
    LOGININFO loginInfo2 = loginInfo;
    // For testing purposes
    loginInfo.PrintLoginInfo(); // For testing purposes
    printf("Players found:  ");
    for(unsigned int i=0; i<loginInfo.playersFound.size(); i++){
        printf("%d ", loginInfo.playersFound[i]);
    }
    printf("\n");

    /* Sending login info to server (and receiving confirmation back) */
    loginInfo2 = Talk2ServerLogin(loginInfo);

    #ifdef DEBUG
        loginInfo2.PrintLoginInfo();
        printf("Players found:  ");
        for(unsigned int i=0; i<loginInfo2.playersFound.size(); i++){
            printf("%d ", loginInfo2.playersFound[i]);
        }
        printf("\n");

        printf("Data has been sent to the server!\n");
    #endif

    return loginInfo2;

}

GAMESTATE Talk2ServerGameState(GAMESTATE gameState){
    int n;

    /* Creating buffer, sending it to server */
    BUF RecvBuf(2000); /* message buffer for receiving a message -- think the max for login and game is well below 2000 bytes, but using this just in case */
    BUF SendBuf; /* message buffer for sending a response */

    SendBuf = createBuffer(gameState);
    printf("Dummy game state:  \n");
    officialGameState.PrintGameState();

    n = write(SocketFD, SendBuf.data(), SendBuf.size());
    if (n < 0){   
        FatalError("writing to socket failed");
    }
    
    /* Getting response from server and closing the socket */
    n = read(SocketFD, RecvBuf.data(), 2000);
    if (n < 0) {   
        FatalError("reading from socket failed");
    }
    RecvBuf.resize(n);

    // Wrapup:  Getting new structure and returning it

    gameState = parsingGameArguments(RecvBuf);
    return gameState;

}


/* MAIN FUNCTION */
int main(int argc, char *argv[]){
    int PortNo;
    struct hostent *Server;

    Program = argv[0];

    if (argc < 3){  /* DEFAULT CASE:  Prompt user for server */ 
        //fprintf(stderr, "Usage: %s hostname port\n", Program);
	    //exit(10);
        char actualServer[100];
        printf("Before starting the login menu, please specify where the server actually is (e.g. an IP address; \'bondi\'; etc):  ");
        scanf(" %s", actualServer);

        Server = gethostbyname(actualServer);
        PortNo = DEFAULT_SERVERPORT;
        
    } else{ /* NON-DEFAULT CASE */
        Server = gethostbyname(argv[1]);
        PortNo = atoi(argv[2]);
    }


    if (Server == NULL)
    {   fprintf(stderr, "%s: no such host named '%s'\n", Program, argv[1]);
        exit(10);
    }

    if (PortNo <= 2000)
    {   fprintf(stderr, "%s: invalid port number %d, should be >2000\n",
		Program, PortNo);
        exit(10);
    }
    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_port = htons(PortNo);
    ServerAddress.sin_addr = *(struct in_addr*)Server->h_addr_list[0];

    // Creating socket
    // int n;
    //int SocketFD;

    /* Connecting to Server*/
    SocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (SocketFD < 0){   
        FatalError("socket creation failed");
    }
    if (connect(SocketFD, (struct sockaddr*)&ServerAddress,
            sizeof(struct sockaddr_in)) < 0){   
                FatalError("connecting to server failed");
    }


    /* TESTING CODE */
    strcpy(playerLoginInfo.playerName, "Alice");
    playerLoginInfo.playerNum = 0;
    playerLoginInfo.numPlayers = 10;
    playerLoginInfo.isHost = 1;
    strcpy(playerLoginInfo.password, "myPassword");
    playerLoginInfo.playerType = Computer;
    playerLoginInfo.playersFound = {1, 0, 0, 0, 0,  0,0,0,0,0};
    printf("Client:  Alice;  Player number:  0 (1), player socket = %d\n", SocketFD);
    Talk2ServerLogin(playerLoginInfo);

    strcpy(playerLoginInfo.playerName, "Bob");
    playerLoginInfo.isHost = 0;
    playerLoginInfo.playerNum = 5;
    playerLoginInfo.playersFound[5] = 1;
    printf("Client:  Bob;  Player number:  5(6), player socket = %d\n", SocketFD);
    Talk2ServerLogin(playerLoginInfo);

    strcpy(playerLoginInfo.playerName, "Eve");
    playerLoginInfo.playerNum = 9;
    playerLoginInfo.playersFound[9] = 1;
    printf("Client:  Eve;  Player number:  9 (10), player socket = %d\n", SocketFD);
    Talk2ServerLogin(playerLoginInfo);


    // Wrapup
    close(SocketFD);
    
}


/* EOF GTK_ClockClient.c */
