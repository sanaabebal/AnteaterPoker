/* GTK_ClockServer.c: GUI example TCP/IP server with timeout support
 * Author: Rainer Doemer, 5/14/24 (based on simple ClockServer.c)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <gtk/gtk.h>
#include <assert.h>
#include <math.h>


#include "cards.hpp"
#include "data.hpp"
#include "DataTransfer.hpp"

#define DEFAULT_PORT 10080



/* #define DEBUG */	/* be verbose */

/*** global variables ****************************************************/

const char *Program	/* program name for descriptive diagnostics */
	= NULL;
int Shutdown		/* keep running until Shutdown == 1 */
	= 0;

// Adding these globally so they can be modified later
GtkWidget *Window;
GtkWidget *player[10];
// Setup for official login information log and log of players; setup for official gamestate
LOGININFO officialLoginInfo;
GAMESTATE officialGameState;
// officialLoginInfo.isHost = 1; moving this to start of main function
PLAYERS players; // will set size of this later with the host specifications


/*** GUI functions *******************************************************/
void ShutdownClicked(		/* shutdown button was clicked */
	GtkWidget *Widget,
	gpointer Data)
{
    #ifdef DEBUG
        printf("%s: ShutdownClicked callback starting...\n", Program);
    #endif
        Shutdown = 1;	/* initiate server shutdown */
        
    #ifdef DEBUG
        printf("%s: ShutdownClicked callback done.\n", Program);
    #endif
} /* end of ShutdownClicked */


// NEW:  CreateWindow (took old code and modified it to display the desired output)
GtkWidget *CreateWindow(	/* create the server window */
	int *argc,
	char **argv[])
{
    //GtkWidget *Window;

    GtkWidget *VBox;
    GtkWidget *ShutdownButton;
    GtkWidget *playerH;

    // Player information:  Holds player name, socket int if found; otherwise, returns 
    //GtkWidget *playerH, *player1, *player2, *player3, *player4, *player5, *player6, *player7, *player8, *player9, *player10;

    
    /* initialize the GTK libraries */
    gtk_init(argc, argv);


    /* create the main, top level window */
    Window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(Window), "Poker Server View");
    gtk_window_set_default_size(GTK_WINDOW(Window), 260, 280);
    gtk_container_set_border_width (GTK_CONTAINER(Window), 10);

    /* overall vertical arrangement in the window */
    // VBox = gtk_vbox_new(FALSE, 10);
    VBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(Window), VBox);

    /* on the top, put player 1's info; from there, keep going down the list of players*/
    playerH = gtk_label_new("Player info:  ");
    player[0] = gtk_label_new("Player 1:  No player found yet.");
    player[1] = gtk_label_new("Player 2:  No player found yet.");
    player[2] = gtk_label_new("Player 3:  No player found yet.");
    player[3] = gtk_label_new("Player 4:  No player found yet.");
    player[4] = gtk_label_new("Player 5:  No player found yet.");
    player[5] = gtk_label_new("Player 6:  No player found yet.");
    player[6] = gtk_label_new("Player 7:  No player found yet.");
    player[7] = gtk_label_new("Player 8:  No player found yet.");
    player[8] = gtk_label_new("Player 9:  No player found yet.");
    player[9] = gtk_label_new("Player 10:  No player found yet.");

    gtk_container_add(GTK_CONTAINER(VBox), playerH);
    gtk_container_add(GTK_CONTAINER(VBox), player[0]);
    gtk_container_add(GTK_CONTAINER(VBox), player[1]);
    gtk_container_add(GTK_CONTAINER(VBox), player[2]);
    gtk_container_add(GTK_CONTAINER(VBox), player[3]);
    gtk_container_add(GTK_CONTAINER(VBox), player[4]);
    gtk_container_add(GTK_CONTAINER(VBox), player[5]);
    gtk_container_add(GTK_CONTAINER(VBox), player[6]);
    gtk_container_add(GTK_CONTAINER(VBox), player[7]);
    gtk_container_add(GTK_CONTAINER(VBox), player[8]);
    gtk_container_add(GTK_CONTAINER(VBox), player[9]);


    /* on the bottom, a button to shutdown the server and quit */
    ShutdownButton = gtk_button_new_with_label("Shutdown Server and Quit");
    gtk_container_add(GTK_CONTAINER(VBox), ShutdownButton);

    /* make sure that everything becomes visible */
    gtk_widget_show_all(Window);

    /* connect window-close with function terminating this server */
    g_signal_connect(Window, "destroy",
			G_CALLBACK(ShutdownClicked), NULL);

    /* connect shutdown button with function terminating this server */
    g_signal_connect(ShutdownButton, "clicked",
                        G_CALLBACK(ShutdownClicked), NULL);

    return(Window);
} /* end of CreateWindow */

// NEW:  UpdatePlayer (completely new code)
void UpdatePlayer(LOGININFO loginInfo, int playerSocket){
    char text[200];
    int n = loginInfo.playerNum;
    if(n < 0 || n >= 10){
        printf("ERROR:  Tried to update player, but player number is out of range.\n");
        return;
    }
    sprintf(text, "Player %d:  Name=%s, Socket Number Used=%d", n+1, loginInfo.playerName, playerSocket);
    gtk_label_set_text(GTK_LABEL(player[n]), text);

}


void UpdateWindow(void)		/* render the window on screen */
{
   /* this server has it's own main loop for handling client connections;
    * as such, it can't have the usual GUI main loop (gtk_main);
    * instead, we call this UpdateWindow function on regular basis
    */
    while(gtk_events_pending())
    {
	gtk_main_iteration();
    }
} /* end of UpdateWindow */



/*** global functions ****************************************************/
void ProcessLoginRequest(int DataSocketFD, BUF RecvBuf);
void ProcessGameStateRequest(int DataSocketFD, BUF RecvBuf);


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

int MakeServerSocket(		/* create a socket on this server */
	uint16_t PortNo)
{
    int ServSocketFD;
    struct sockaddr_in ServSocketName;

    /* create the socket */
    ServSocketFD = socket(PF_INET, SOCK_STREAM, 0);
    if (ServSocketFD < 0){   
        FatalError("service socket creation failed");
    }
    /* bind the socket to this server */
    ServSocketName.sin_family = AF_INET;
    ServSocketName.sin_port = htons(PortNo);
    ServSocketName.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(ServSocketFD, (struct sockaddr*)&ServSocketName,
		sizeof(ServSocketName)) < 0){
            FatalError("binding the server to a socket failed");
    }
    /* start listening to this socket */
    if (listen(ServSocketFD, 5) < 0)	/* max 5 clients in backlog */{
        FatalError("listening on socket failed");
    }

    return ServSocketFD;
} /* end of MakeServerSocket */


void ProcessClientRequest(int DataSocketFD){
    BUF RecvBuf(2000); /* message buffer for receiving a message -- think the max for login and game is well below 2000 bytes, but using this just in case */
    ssize_t n = 0;

    n = read(DataSocketFD, RecvBuf.data(), RecvBuf.size());
        if (n < 0) 
        {   FatalError("reading from data socket failed");
        } else if(n == 0){ // Client closed the connection
            return;
        }
        RecvBuf.resize(n);
    
    int packageType;
    memcpy(&packageType, &RecvBuf[0], sizeof(int));

    if(packageType == login){
        ProcessLoginRequest(DataSocketFD, RecvBuf);
    } else if(packageType = game){
        ProcessGameStateRequest(DataSocketFD, RecvBuf);
    } else{
        FatalError("Message received was neither logininfo or gamestate.");
    }
    
    return;
}

// NEW:  ProcessLoginRequest (built on old code, but)
void ProcessLoginRequest(		/* process a LOGININFO request by a client --user will send one w/ dummy values, if first one, one with real values the second time */
	int DataSocketFD, BUF RecvBuf)
{
        static int foundHost = 0;
        ssize_t n = 0;
        
        BUF SendBuf; /* message buffer for sending a response */
    
        
        LOGININFO loginInfo;

    
    // Handling processed user input
        loginInfo = parsingLoginArguments(RecvBuf);
        SendBuf = createBuffer(officialLoginInfo);
        if(loginInfo.playerNum == -1){ // dummy value--first time the user is connecting to the server
            // Do nothing to the official structure yet--just send prompt structure
            n = write(DataSocketFD, SendBuf.data(), SendBuf.size());
            if(n < 0){
                FatalError("Writing data to socket failed");
            }

            return; // client will have to try again to communicate w/ server, but will now have the data needed to adequately do so
        } 
        // user sent an actual value--update the official structure, record the player
            if(loginInfo.isHost && !foundHost){ // player is host--update the login structure, PLAYERS list accordingly
                players.resize(loginInfo.numPlayers); // client does NOT do this

                officialLoginInfo.numPlayers = 0;
                // officialLoginInfo.playersFound.resize(loginInfo.numPlayers); -- assume the client does this (and inserts themselves correctly in the list)
                
                strcpy(officialLoginInfo.password, loginInfo.password);

                // Wrapup:  Setting isHost to 0 and foundHost to 1 so this is only ever triggered once
                officialLoginInfo.isHost = 0;
                foundHost = 1;
            }
            // Updating shared aspects btw host and non-host
            officialLoginInfo.playersFound = loginInfo.playersFound;
            officialLoginInfo.numPlayers = loginInfo.numPlayers;


            // Record the player
            PLAYER newPlayer;
            newPlayer.playerNum = loginInfo.playerNum; strcpy(newPlayer.name, loginInfo.playerName); newPlayer.playerType = loginInfo.playerType;
            newPlayer.playerSocket = DataSocketFD;
            if(loginInfo.playerNum >= 0 && loginInfo.playerNum < (int)players.size()){ // need typecast to avoid warnings
                players[loginInfo.playerNum] = newPlayer;
            } else{
                FatalError("Invalid player number received");
            }
            
            // Wrapup
            UpdatePlayer(loginInfo, DataSocketFD);

            // Sending updated copy to user
            SendBuf = createBuffer(officialLoginInfo);
            n = write(DataSocketFD, SendBuf.data(), SendBuf.size());     
} /* end of ProcessRequest */

void ProcessGameStateRequest(int DataSocketFD, BUF RecvBuf){
    return;

}


// Updated post-alpha
void ServerMainLoop(		/* simple server main loop */
	int ServSocketFD,		/* server socket to wait on */
	int Timeout)			/* timeout in micro seconds */
{
            int DataSocketFD;	/* socket for a new client */
            socklen_t ClientLen;
            struct sockaddr_in
            ClientAddress;	/* client address we connect with */
            fd_set ActiveFDs;	/* socket file descriptors to select from */
            fd_set ReadFDs;	/* socket file descriptors ready to read from */
            struct timeval TimeVal;
            int res, i;

            FD_ZERO(&ActiveFDs);		/* set of active sockets */
            FD_SET(ServSocketFD, &ActiveFDs);	/* server socket is active */

            #ifdef TESTING
                officialLoginInfo.playerNum = 2;
                strcpy(officialLoginInfo.playerName, "Peter");
                UpdatePlayer(officialLoginInfo, 0);
            #endif


            while(!Shutdown){
                UpdateWindow();	/* update the GUI window */
                ReadFDs = ActiveFDs;
                TimeVal.tv_sec  = Timeout / 1000000;
                TimeVal.tv_usec = Timeout % 1000000;
                /* block until input arrives on active sockets or until timeout */
                res = select(FD_SETSIZE, &ReadFDs, NULL, NULL, &TimeVal);
                if (res < 0)
                {   FatalError("wait for input or timeout (select) failed");
                }
                if (res == 0)	/* timeout occurred */
                {
                    //DisplayCurrentTime();
                }
                else		/* some FDs have data ready to read */
                {   for(i=0; i<FD_SETSIZE; i++)
                    {   if (FD_ISSET(i, &ReadFDs))
                    {   if (i == ServSocketFD)
                        {	/* connection request on server socket */
                #ifdef DEBUG
                            printf("%s: Accepting new client...\n", Program);
                #endif
                        ClientLen = sizeof(ClientAddress);
                        DataSocketFD = accept(ServSocketFD, (struct sockaddr*)&ClientAddress, &ClientLen);
                        if (DataSocketFD < 0)
                        {   FatalError("data socket creation (accept) failed");
                        }
                #ifdef DEBUG
                            printf("%s: New client connected from %s:%hu.\n",
                                Program,
                                inet_ntoa(ClientAddress.sin_addr),
                                ntohs(ClientAddress.sin_port));
                #endif
                        FD_SET(DataSocketFD, &ActiveFDs);
                        }
                        else
                        {   /* active communication with a client */
                #ifdef DEBUG
                            printf("%s: Dealing with client on FD%d...\n",
                                Program, i);
                #endif
                        ProcessClientRequest(i);


                        //close(i); -- Update 5/27, after alpha
                        // FD_CLR(i, &ActiveFDs); -- Update after alpha
                        }
                    }
                    }
                }
            }
} /* end of ServerMainLoop */

/*** main function *******************************************************/

int main(			/* the main function */
	int argc,
	char *argv[])
{
    // Doing this to set isHost to be 1 (i.e. first person to log on will be the host)
    officialLoginInfo.isHost = 1;

    int ServSocketFD;	/* socket file descriptor for service */
    int PortNo;		/* port number */
    GtkWidget *Window;	/* the server window */

    Program = argv[0];	/* publish program name (for diagnostics) */
    #ifdef DEBUG
        printf("%s: Starting...\n", Program);
    #endif
    if (argc < 2){   /* NEW:  DEFAULT CASE */
        //fprintf(stderr, "Usage: %s port\n", Program);
	    //exit(10);
        PortNo = DEFAULT_PORT;
    } else{ /* NEW:  NON-DEFAULT CASE */
        PortNo = atoi(argv[1]);	/* get the port number */
    }
    
    if (PortNo <= 2000)
    {   fprintf(stderr, "%s: invalid port number %d, should be >2000\n",
		Program, PortNo);
        exit(10);
    }
    #ifdef DEBUG
        printf("%s: Creating the server socket...\n", Program);
    #endif
    ServSocketFD = MakeServerSocket(PortNo);

    #ifdef DEBUG
        printf("%s: Creating the server window...\n", Program);
    #endif
    Window = CreateWindow(&argc, &argv);
    if (!Window)
    {   fprintf(stderr, "%s: cannot create GUI window\n", Program);
        exit(10);
    }
    #ifdef DEBUG
        printf("%s: Providing current time at port %d...\n", Program, PortNo);
    #endif
    ServerMainLoop(ServSocketFD, 250000);
    #ifdef DEBUG
        printf("\n%s: Shutting down.\n", Program);
    #endif
    close(ServSocketFD);
    return 0;
} /* end of main */

/* EOF GTK_ClockServer.c */
