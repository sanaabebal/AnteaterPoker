// serverGUI.cpp : Defines the entry point for the application.
// Author: Sanaa Bebal
/* Modifications:
    06/02/26: Combining all code from beta version and my own work
    06/03/26
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
// CODEX FIX: server launch/action handling now uses the real round setup and turn-advance logic.
#include "gamestate.hpp"

using namespace std;

#define DEFAULT_PORT 10080

/* #define DEBUG */	/* be verbose */

/*** global variables ****************************************************/

const char *Program	/* program name for descriptive diagnostics */
	= NULL;
int Shutdown		/* keep running until Shutdown == 1 */
	= 0;
int gameStarted = 0; // used to track whether in login stage or gamestate stage

// Adding these globally so they can be modified later
GtkWidget *Window;
GtkWidget *player[10];
GtkWidget *playerRows[10];
// Setup for official login information log and log of players; setup for official gamestate
LOGININFO officialLoginInfo;
GAMESTATE officialGameState;
// officialLoginInfo.isHost = 1; moving this to start of main function
PLAYERS players; // will set size of this later with the host specifications
PILE refDeck; // reference deck (initialized in main)

// CODEX FIX: use named lobby markers so launch/status requests are separate from real game rounds.
static const int LOBBY_ROSTER_REQUEST = -1;
// CODEX FIX: marker received when the host clicks Launch New Session.
static const int LOBBY_START_REQUEST = -2;
// CODEX FIX: marker received when a joined client asks if the host has launched.
static const int LOBBY_STATUS_REQUEST = -3;
// CODEX FIX: marker received when a client asks for the latest running game state.
static const int GAME_STATE_REFRESH_REQUEST = -4;
// CODEX FIX: marker received when the dealer asks to start the next hand after a completed hand.
static const int DEALER_START_HAND_REQUEST = -5;


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

// adding in helper functions for design purposes
void activePlayers(int totalSeats) {
    // checking if the sent number of seats is valid within the game
    if (totalSeats <= 0 || totalSeats > 10){
        return;
    }
    // checking what seats should be visible or not
    for (int i = 0; i < 10; i++){
        if (i < totalSeats){
            gtk_widget_set_visible(playerRows[i], TRUE);
        }
        // anything after number of seats won't be visible
        else {
            gtk_widget_set_visible(playerRows[i], FALSE);
        }
    }

}


// NEW:  CreateWindow (took old code and modified it to display the desired output)
GtkWidget *CreateWindow (int *argc, char **argv[])
{
    //GtkWidget *Window;

    GtkWidget *VBox;
    GtkWidget *ShutdownButton;
    GtkWidget *playerH;
    GtkCssProvider *Provider;
    GtkStyleContext *Context;

    // Player information:  Holds player name, socket int if found; otherwise, returns 
    //GtkWidget *playerH, *player1, *player2, *player3, *player4, *player5, *player6, *player7, *player8, *player9;

    
    /* initialize the GTK libraries */
    gtk_init(argc, argv);


    /* create the main, top level window */
    Window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(Window), "Poker Server Dashboard");
    gtk_window_set_default_size(GTK_WINDOW(Window), 350, 450);
    gtk_container_set_border_width (GTK_CONTAINER(Window), 15);

    // styling for the server screen

    Provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(Provider,
        /* CODEX FIX: added real server-side CSS so the server window matches the styled client side. */
        ".server-root, .main-window { background-color: #2b4c54; }\n"
        ".header-label { color: #f5ecc8; font-family: Georgia, serif; font-size: 18px; font-weight: bold; }\n"
        ".player-card { background-color: #244348; border: 1px solid #7f956f; padding: 6px; }\n"
        ".player-label { color: #f5ecc8; font-family: Georgia, serif; font-size: 13px; }\n"
        ".shutdown-button { background-image: none; background-color: #e8d9a0; color: #2c1f0e; font-family: Georgia, serif; font-size: 13px; border: 2px solid #a08040; padding: 8px 18px; }\n"
        ".shutdown-button label { color: #2c1f0e; }\n"
        ".shutdown-button:hover { background-image: none; background-color: #f0e6b8; border-color: #c0a050; }\n",
        -1, NULL );

    Context = gtk_widget_get_style_context(Window);
    // CODEX FIX: attach the class used by the server CSS above.
    gtk_style_context_add_class(Context, "main-window");
    gtk_style_context_add_provider(Context, GTK_STYLE_PROVIDER(Provider),
                                     GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    // CODEX FIX: install the CSS for all child widgets, not just the top-level window context.
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                     GTK_STYLE_PROVIDER(Provider),
                                     GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    

    /* overall vertical arrangement in the window */
    // VBox = gtk_vbox_new(FALSE, 10);
    VBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    // CODEX FIX: make the server root box use the same visible background as the window.
    gtk_style_context_add_class(gtk_widget_get_style_context(VBox), "server-root");
    gtk_container_add(GTK_CONTAINER(Window), VBox);

    /* on the top, put player 1's info; from there, keep going down the list of players*/
    playerH = gtk_label_new("Player info:  ");
    // CODEX FIX: make the server heading readable against the teal background.
    gtk_style_context_add_class(gtk_widget_get_style_context(playerH), "header-label");
    gtk_box_pack_start(GTK_BOX(VBox), playerH, FALSE, FALSE, 5);

    // building out potential seats for selected number of students

    for (int i = 0; i < 10; i++) {
        char labelBuffer[50];
        sprintf(labelBuffer, "Player %d: No player found yet.", i + 1);

        playerRows[i] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_style_context_add_provider(gtk_widget_get_style_context(playerRows[i]),
                                        GTK_STYLE_PROVIDER(Provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        gtk_style_context_add_class(gtk_widget_get_style_context(playerRows[i]), "player-card");


        player[i] = gtk_label_new(labelBuffer);
        // CODEX FIX: style each server player label so inactive/active seats are readable.
        gtk_style_context_add_class(gtk_widget_get_style_context(player[i]), "player-label");
        gtk_box_pack_start(GTK_BOX(playerRows[i]), player[i], TRUE, TRUE, 5);

        gtk_box_pack_start(GTK_BOX(VBox), playerRows[i], FALSE, FALSE, 0);

        gtk_widget_set_no_show_all(playerRows[i], TRUE);
        gtk_widget_hide(playerRows[i]);
    }



    /* on the bottom, a button to shutdown the server and quit */
    ShutdownButton = gtk_button_new_with_label("Shutdown Server and Quit");
    //gtk_container_add(GTK_CONTAINER(VBox), ShutdownButton);
    // CODEX FIX: style the server shutdown button instead of leaving the default grey widget.
    gtk_style_context_add_class(gtk_widget_get_style_context(ShutdownButton), "shutdown-button");
    gtk_style_context_add_provider(gtk_widget_get_style_context(ShutdownButton),
                                    GTK_STYLE_PROVIDER(Provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    gtk_box_pack_end(GTK_BOX(VBox), ShutdownButton, FALSE, FALSE, 10);

    /* make sure that everything becomes visible */
    gtk_widget_show_all(Window);

    /* connect window-close with function terminating this server */
    g_signal_connect(Window, "destroy",
			G_CALLBACK(ShutdownClicked), NULL);

    /* connect shutdown button with function terminating this server */
    g_signal_connect(ShutdownButton, "clicked",
                        G_CALLBACK(ShutdownClicked), NULL);

    g_object_unref(Provider);
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


void MessageAllClients(){
    streamsize n;
    BUF SendBuf;
    int clientSocket;

    SendBuf = createBuffer(officialGameState);

    for(unsigned int i=0; i<officialGameState.players.size(); i++){
        clientSocket = officialGameState.players[i].playerSocket;

        n = write(clientSocket, SendBuf.data(), SendBuf.size());
        if(n < 0){
            printf("ERROR:  Couldn't write message to player %d (0, 1, 2, etc.)\n", i);
        }
    }
    return;
}

// CODEX FIX: check whether every hosted seat has a confirmed player before launch succeeds.
static int LobbyHasAllPlayers(){
    if(officialLoginInfo.numPlayers <= 0 || officialLoginInfo.playersFound.empty()){
        return 0;
    }

    for(unsigned int i=0; i<officialLoginInfo.playersFound.size(); i++){
        if(officialLoginInfo.playersFound[i] == 0){
            return 0;
        }
    }

    return 1;
}

// CODEX FIX: mirror login-stage registered players into GAMESTATE for lobby refresh/status replies.
static void SyncLobbyRosterIntoGameState(){
    officialGameState.numPlayers = officialLoginInfo.numPlayers;
    officialGameState.players = players;
}

// CODEX FIX: write a GAMESTATE response for every lobby sync request so clients do not block.
static void WriteGameStateResponse(int DataSocketFD, const char* errorMessage){
    streamsize n = 0;
    BUF SendBuf = createBuffer(officialGameState);

    n = write(DataSocketFD, SendBuf.data(), SendBuf.size());
    if(n < 0){
        FatalError(errorMessage);
    }
}

// CODEX FIX: start the first real hand with shuffled/dealt cards and blind/turn state.
static void StartFirstHand(){
    GAMESTATE startingState;
    // CODEX FIX: use the server-owned registered players as the canonical game roster.
    startingState.players = players;
    // CODEX FIX: numPlayers must match the roster before startRound() posts blinds/deals.
    startingState.numPlayers = startingState.players.size();
    // CODEX FIX: startRound() advances the dealer, so seed it to the previous seat for player 1 to deal first.
    startingState.dealerPlayer = (startingState.numPlayers > 0) ? startingState.numPlayers - 1 : 0;
    // CODEX FIX: startRound() shuffles, deals hole/community piles, posts blinds, and sets playerTurn.
    officialGameState = startRound(startingState);
    // CODEX FIX: remember that clients should now render/poll the running game instead of the lobby.
    gameStarted = 1;
}

int ProcessClientRequest(int DataSocketFD){
    BUF RecvBuf(2000); /* message buffer for receiving a message -- think the max for login and game is well below 2000 bytes, but using this just in case */
    streamsize n = 0;

    n = read(DataSocketFD, RecvBuf.data(), RecvBuf.size());
        if (n < 0) 
        {   FatalError("reading from data socket failed");
            return -1;
        } else if(n == 0){ // Client closed the connection
            return -1; 
        }
        RecvBuf.resize(n);
    
    int packageType;
    memcpy(&packageType, &RecvBuf[0], sizeof(int));

    if(packageType == login){
        ProcessLoginRequest(DataSocketFD, RecvBuf);
    } else if(packageType == game){
        ProcessGameStateRequest(DataSocketFD, RecvBuf);
    } else{
        FatalError("Message received was neither logininfo or gamestate.");
        return -1;
    }
    
    return 0;
}

// NEW:  ProcessLoginRequest (built on old code, but)
void ProcessLoginRequest(		/* process a LOGININFO request by a client --user will send one w/ dummy values, if first one, one with real values the second time */
	int DataSocketFD, BUF RecvBuf)
{
        static int foundHost = 0;
        streamsize n = 0;
        
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

                officialLoginInfo.numPlayers = loginInfo.numPlayers;
                // CODEX FIX: server owns the occupancy list after the host creates the room.
                officialLoginInfo.playersFound = loginInfo.playersFound;

                strcpy(officialLoginInfo.password, loginInfo.password);

                // adding in the active seats of players
                activePlayers(loginInfo.numPlayers);


                // Wrapup:  Setting isHost to 0 and foundHost to 1 so this is only ever triggered once
                officialLoginInfo.isHost = 0;
                foundHost = 1;
            } else if(loginInfo.isHost && foundHost){
                // CODEX FIX: reject duplicate host submissions instead of letting them overwrite the room.
                SendBuf = createBuffer(officialLoginInfo);
                n = write(DataSocketFD, SendBuf.data(), SendBuf.size());
                if(n < 0){
                    FatalError("Writing data to socket failed");
                }
                return;
            } else{
                if(!foundHost || officialLoginInfo.numPlayers <= 0){
                    // CODEX FIX: reject joins before a host has created the room.
                    SendBuf = createBuffer(officialLoginInfo);
                    n = write(DataSocketFD, SendBuf.data(), SendBuf.size());
                    if(n < 0){
                        FatalError("Writing data to socket failed");
                    }
                    return;
                }
                if(strcmp(loginInfo.password, officialLoginInfo.password) != 0){
                    // CODEX FIX: enforce the saved room password on the server too.
                    printf("Rejected join for player %d: incorrect password.\n", loginInfo.playerNum + 1);
                    SendBuf = createBuffer(officialLoginInfo);
                    n = write(DataSocketFD, SendBuf.data(), SendBuf.size());
                    if(n < 0){
                        FatalError("Writing data to socket failed");
                    }
                    return;
                }
                if(loginInfo.playerNum < 0 || loginInfo.playerNum >= (int)officialLoginInfo.playersFound.size()
                   || officialLoginInfo.playersFound[loginInfo.playerNum] != 0){
                    // CODEX FIX: reject invalid or occupied slots instead of accepting stale client state.
                    printf("Rejected join for player %d: slot unavailable.\n", loginInfo.playerNum + 1);
                    SendBuf = createBuffer(officialLoginInfo);
                    n = write(DataSocketFD, SendBuf.data(), SendBuf.size());
                    if(n < 0){
                        FatalError("Writing data to socket failed");
                    }
                    return;
                }
                // CODEX FIX: mark only the confirmed joining slot, instead of trusting the client's whole playersFound vector.
                officialLoginInfo.playersFound[loginInfo.playerNum] = loginInfo.playerType;
                // CODEX FIX: keep numPlayers server-owned after host setup.
                loginInfo.numPlayers = officialLoginInfo.numPlayers;
            }


            // Record the player
            PLAYER newPlayer;
            newPlayer.playerNum = loginInfo.playerNum; 
            strcpy(newPlayer.name, loginInfo.playerName); 
            newPlayer.playerType = loginInfo.playerType;
            newPlayer.playerSocket = DataSocketFD;
            
            if(loginInfo.playerNum >= 0 && loginInfo.playerNum < (int)players.size()){ // need typecast to avoid warnings
                players[loginInfo.playerNum] = newPlayer;
            } else{
                FatalError("Invalid player number received");
            }
            // CODEX FIX: keep the GAMESTATE roster mirror current for host lobby refresh requests.
            officialGameState.numPlayers = officialLoginInfo.numPlayers;
            // CODEX FIX: copy registered server-side players into the roster mirror after each accepted login.
            officialGameState.players = players;

            // Wrapup
            UpdatePlayer(loginInfo, DataSocketFD);

            // Sending updated copy to user
            SendBuf = createBuffer(officialLoginInfo);
            n = write(DataSocketFD, SendBuf.data(), SendBuf.size());     
} /* end of ProcessRequest */

void ProcessGameStateRequest(int DataSocketFD, BUF RecvBuf){
    GAMESTATE gameState;

    // Handling processed user input
    gameState = parsingGameArguments(RecvBuf);

    if(gameState.numPlayers == LOBBY_ROSTER_REQUEST){
        // CODEX FIX: respond to read-only lobby roster requests from the host client.
        SyncLobbyRosterIntoGameState();
        WriteGameStateResponse(DataSocketFD, "Writing roster data to socket failed");
        return;
    }

    if(gameState.numPlayers == LOBBY_START_REQUEST){
        // CODEX FIX: host launch is now a server-owned state change, not just a local screen switch.
        SyncLobbyRosterIntoGameState();
        if(LobbyHasAllPlayers()){
            printf("ALL PLAYERS HAVE LOGGED IN!  STARTING GAME...\n");
            // CODEX FIX: create the first real hand instead of returning a lobby placeholder.
            StartFirstHand();
        } else{
            // CODEX FIX: leave allCards empty so the host knows the game is not ready yet.
            officialGameState.allCards.clear();
        }
        WriteGameStateResponse(DataSocketFD, "Writing launch response to socket failed");
        return;
    }

    if(gameState.numPlayers == LOBBY_STATUS_REQUEST){
        // CODEX FIX: joined clients poll this branch while waiting for the host launch.
        if(!gameStarted){
            // CODEX FIX: while waiting, return roster-only state with empty cards.
            SyncLobbyRosterIntoGameState();
            // CODEX FIX: empty cards are the waiting signal for joined clients.
            officialGameState.allCards.clear();
        }
        WriteGameStateResponse(DataSocketFD, "Writing launch status to socket failed");
        return;
    }

    if(gameState.numPlayers == GAME_STATE_REFRESH_REQUEST){
        // CODEX FIX: running clients poll this branch to stay synchronized after launch.
        WriteGameStateResponse(DataSocketFD, "Writing game refresh response to socket failed");
        return;
    }

    if(gameState.numPlayers == DEALER_START_HAND_REQUEST){
        if(gameStarted && officialGameState.playerTurn < 0
           && gameState.playerTurn == officialGameState.dealerPlayer){
            // CODEX FIX: only the current dealer can advance the paused completed-hand state into a newly dealt hand.
            officialGameState = startRound(officialGameState);
        }
        // CODEX FIX: always reply so rejected/non-dealer start attempts do not block the client.
        WriteGameStateResponse(DataSocketFD, "Writing dealer start-hand response to socket failed");
        return;
    }

    if(gameState.numPlayers == 0){ // dummy value--first time the user is connecting to the server regarding this
        // CODEX FIX: keep the legacy dummy-game request from blocking if the lobby is not ready.
        SyncLobbyRosterIntoGameState();
        if(LobbyHasAllPlayers()){
            printf("ALL PLAYERS HAVE LOGGED IN!  STARTING GAME...\n");
            // CODEX FIX: legacy start requests should create the same real first hand as the host button.
            StartFirstHand();
        } else{
            // CODEX FIX: legacy start requests now receive a waiting response instead of no response.
            officialGameState.allCards.clear();
        }
        WriteGameStateResponse(DataSocketFD, "Writing legacy game-start response to socket failed");
        return;
    }

    if(gameStarted){
        // CODEX FIX: apply a player's action through the shared game-state updater.
        officialGameState = updateGameState(gameState);
        // CODEX FIX: immediately reply so the acting client sees the advanced turn/state.
        WriteGameStateResponse(DataSocketFD, "Writing game action response to socket failed");
        return;
    }

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
                            printf("%s: Dealing with client on FD%d...\n", Program, i);
                #endif

                        int status = ProcessClientRequest(i);
                // Actual control flow logic
                            if (status < 0) {
                                #ifndef DEBUG 
                                    printf("%s: Client on FD%d has disconnected. Clearing up descripto.\n", Program, i);
                                #endif
                                close(i);
                                FD_CLR(i, &ActiveFDs);
                            }
                            else {
                                if (!gameStarted) {

                                }
                                else {

                                }
                            }


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
    refDeck = initDeck();
    srand(time(NULL)); // using this for random number generation (needed for shuffling algorithm)

    int ServSocketFD;	/* socket file descriptor for service */
    int PortNo;		/* port number */
    //GtkWidget *Window;	/* the server window */

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
