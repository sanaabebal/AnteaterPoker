/* clientTextW8.cpp */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
//#include <gtk/gtk.h>
#include <assert.h>

#include "cards.hpp"
#include "data.hpp"
#include "DataTransfer.hpp"

/*** global variables ****************************************************/

const char *Program	/* program name for descriptive diagnostics */
	= NULL;
struct sockaddr_in
	ServerAddress;	/* server address we connect with */



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
    int SocketFD;

    /* Connecting to Server*/
    SocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (SocketFD < 0){   
        FatalError("socket creation failed");
    }
    if (connect(SocketFD, (struct sockaddr*)&ServerAddress,
            sizeof(struct sockaddr_in)) < 0){   
                FatalError("connecting to server failed");
    }

    /* Creating buffer, sending it to server */
    BUF RecvBuf(2000); /* message buffer for receiving a message -- think the max for login and game is well below 2000 bytes, but using this just in case */
    BUF SendBuf; /* message buffer for sending a response */

    SendBuf = createBuffer(loginInfo);
    n = write(SocketFD, SendBuf.data(), 2000);
    if (n < 0){   
        FatalError("writing to socket failed");
    }
    
    /* Getting response from server and closing the socket */
    n = read(SocketFD, RecvBuf.data(), 2000);
    if (n < 0) {   
        FatalError("reading from socket failed");
    }
    close(SocketFD);

    // Wrapup:  Getting new structure and returning it
    loginInfo = parsingLoginArguments(RecvBuf);
    return loginInfo;
} /* end of Talk2Server */


void NameWindow(int *argc, char **argv[], LOGININFO &loginInfo){
    char name[41];
    printf("Please enter your name (40 characters max, no special characters):  ");
    scanf(" %s", name);

    // IMPORTANT
    strcpy(loginInfo.playerName, name);
}

void PasswordWindow(int *argc, char **argv[], LOGININFO &loginInfo){
    char password[41];
    if(loginInfo.isHost){
        printf("Please create a password for the room (40 characters max, no special characters):  ");
        scanf(" %s", password);
    } else{
        int count = 0;
        do{
            if(count > 0){
                printf("Not a recognized room password.  Please try again.\n");
            }
            printf("Please enter the password for the room:  ");
            scanf(" %s", password);
        } while(strcmp(password, loginInfo.password) != 0);
    }

    // IMPORTANT
    strcpy(loginInfo.password, password);
}

void NumPlayersWindow(int *argc, char **argv[], LOGININFO &loginInfo){
    int numPlayers = loginInfo.numPlayers;

    if(loginInfo.isHost){
        if(loginInfo.numPlayers != 0){
            printf("Hmm...host's setting the number of players, but it seems like someone else already has?\n");
        }

        int count = 0;
        do{
            if(count > 0){ printf("Not a valid number of players.  Please try again.\n"); }
            printf("Please enter the number of players expected (between 2 and 10, inclusively):  ");
            scanf(" %d", &numPlayers);
        }while(numPlayers < 2 || numPlayers > 10);

        loginInfo.playersFound.resize(numPlayers); // IMPORTANT!
    } else{
        printf("There are %d players allowed in this game.\n", loginInfo.numPlayers);
    }

    // IMPORTANT!
    loginInfo.numPlayers = numPlayers;
}

void PlayerNumWindow(int *argc, char **argv[], LOGININFO &loginInfo){
    int playerNum = -1;

    int count = 0;
    do{
        if(count > 0){
            printf("Not an allowed position.  Please try again.\n");
        }
        printf("Please enter your player number/position (note that numbering starts at 0).");
        printf("Available positions:  ");
        for(unsigned int i=0; i<loginInfo.playersFound.size(); i++){
            if(loginInfo.playersFound[i] == 0){
                printf(" %d", i);
            }
        }
        printf("\nYour choice:  ");
        scanf(" %d", &playerNum);
    } while(playerNum < 0 || playerNum >= loginInfo.playersFound.size() || loginInfo.playersFound[playerNum] != 0);

    // IMPORTANT!
    loginInfo.playerNum = playerNum;
    loginInfo.playersFound[playerNum] = 1;
}

void PlayerTypeWindow(int *argc, char **argv[], LOGININFO &loginInfo){
    int playerType = 0;
    printf("Human (type \'1\') or Computer (type \'2\')?  (Default is human):  ");
    scanf(" %d", &playerType);
    playerType = (playerType == 2) ? Computer : Human;

    // IMPORTANT!
    loginInfo.playerType = playerType;
}


void MainMenu(int *argc, char **argv[], LOGININFO &loginInfo){
    NameWindow(argc, argv, loginInfo);
    PasswordWindow(argc, argv, loginInfo);
    NumPlayersWindow(argc, argv, loginInfo);
    PlayerNumWindow(argc, argv, loginInfo);
    PlayerTypeWindow(argc, argv, loginInfo);
}

int main(int argc, char *argv[]){
    int PortNo;
    struct hostent *Server;

    Program = argv[0];

    if (argc < 3)
    {   fprintf(stderr, "Usage: %s hostname port\n", Program);
	exit(10);
    }
    Server = gethostbyname(argv[1]);
    if (Server == NULL)
    {   fprintf(stderr, "%s: no such host named '%s'\n", Program, argv[1]);
        exit(10);
    }
    PortNo = atoi(argv[2]);
    if (PortNo <= 2000)
    {   fprintf(stderr, "%s: invalid port number %d, should be >2000\n",
		Program, PortNo);
        exit(10);
    }
    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_port = htons(PortNo);
    ServerAddress.sin_addr = *(struct in_addr*)Server->h_addr_list[0];


    /* LOGIN MENU */
    LOGININFO loginInfo;
    loginInfo = Talk2ServerLogin(loginInfo);
    MainMenu(&argc, &argv, loginInfo);
    // For testing purposes
    loginInfo.PrintLoginInfo(); // For testing purposes
    printf("Players found:  ");
    for(unsigned int i=0; i<loginInfo.playersFound.size(); i++){
        printf("%d ", loginInfo.playersFound[i]);
    }
    printf("\n");

    /* Sending login info to server (and receiving confirmation back) */
    Talk2ServerLogin(loginInfo);

    loginInfo.PrintLoginInfo();
    printf("Players found:  ");
    for(unsigned int i=0; i<loginInfo.playersFound.size(); i++){
        printf("%d ", loginInfo.playersFound[i]);
    }
    printf("\n");

    printf("Data has been sent to the server!\n");
}