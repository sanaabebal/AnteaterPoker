// clientGUI.cpp

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdexcept>
#include <gtk/gtk.h>
#include <assert.h>
#include <vector>
#include <string>
#include <memory>

#include "cards.hpp"
#include "data.hpp"
#include "DataTransfer.hpp"
#include "clientGUI.hpp"

#define DEFAULT_SERVERPORT 10080
#define DEFAULT_SERVERNAME "zuma"


using namespace std;

// global variables
const char *Program = NULL;
struct sockaddr_in ServerAddress;
int SocketFD = 0;

GAMESTATE officialGameState;
LOGININFO playerLoginInfo;
int officialPlayerSocket = 0;

static const char* SCR_MAIN = "main_menu";
static const char* SCR_HOST = "host";
static const char* SCR_JOIN = "join";
static const char* SCR_GAME = "game";
static const char* SCR_GAMEOVER = "game_over";

const char* ClientGUI::screenName(ScreenID id) {
    switch (id) {
        case ScreenID::Login:
            return SCR_MAIN; 
        case ScreenID::Host:
            return SCR_HOST;
        case ScreenID::Join:
            return SCR_JOIN;
        case ScreenID::Poker:
            return SCR_GAME; 
        case ScreenID::GameOver: 
            return SCR_GAMEOVER;
    }
    return SCR_MAIN;
}

// global functionality -- all to do with communications with the server and client
void FatalError(		/* printing out error diagnostics and abort */
	const char *ErrorMsg)
{
    fputs(Program, stderr);
    fputs(": ", stderr);
    perror(ErrorMsg);
    fputs(Program, stderr);
    fputs(": Exiting!\n", stderr);
    exit(20);
}

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

GAMESTATE ServerGameStateRead(){
    int n;
    GAMESTATE gameState;

    /* Creating buffer, sending it to server */
    BUF RecvBuf(2000); /* message buffer for receiving a message -- think the max for login and game is well below 2000 bytes, but using this just in case */
    BUF SendBuf; /* message buffer for sending a response */
    
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

void ServerGameStateWrite(GAMESTATE gameState){
    int n;

    /* Creating buffer, sending it to server */
    BUF RecvBuf(2000); /* message buffer for receiving a message -- think the max for login and game is well below 2000 bytes, but using this just in case */
    BUF SendBuf; /* message buffer for sending a response */

    SendBuf = createBuffer(gameState);
    //printf("Dummy game state:  \n");
    //officialGameState.PrintGameState();

    n = write(SocketFD, SendBuf.data(), SendBuf.size());
    if (n < 0){   
        FatalError("writing to socket failed");
    }
    

}

ClientGUI::ClientGUI(int argc, char** argv) {
    gtk_init(&argc, &argv);
    buildWindow(argc, argv);
    wireCallbacks();
}

ClientGUI::~ClientGUI() {}

void ClientGUI::run() {
    gtk_widget_show_all(window);
    gtk_stack_set_visible_child_name(GTK_STACK(stack), SCR_MAIN);
    gtk_main();
}

void ClientGUI::show(ScreenID id) { 
    gtk_stack_set_visible_child_name(GTK_STACK(stack), screenName(id));
}

void ClientGUI::buildWindow(int , char** ) {
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Anteater Poker");
    gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    g_signal_connect(window, "destroy", G_CALLBACK(onWindowDestroy), nullptr);

    stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_NONE);
    gtk_container_add(GTK_CONTAINER(window), stack);

    loginScreenObject = make_unique<loginScreen>();
    hostScreenObject = make_unique<hostScreen>();
    joinScreenObject = make_unique<joinScreen>();
    pokerScreenObject = make_unique<pokerScreen>();
    gameOverScreenObject = make_unique<gameOverScreen>();

    gtk_stack_add_named(GTK_STACK(stack), loginScreenObject->getWidget(), SCR_MAIN);
    gtk_stack_add_named(GTK_STACK(stack), hostScreenObject->getWidget(), SCR_HOST);
    gtk_stack_add_named(GTK_STACK(stack), joinScreenObject->getWidget(), SCR_JOIN);
    gtk_stack_add_named(GTK_STACK(stack), pokerScreenObject->getWidget(), SCR_GAME);
    gtk_stack_add_named(GTK_STACK(stack), gameOverScreenObject->getWidget(), SCR_GAMEOVER);
}

void ClientGUI::wireCallbacks() {
    loginScreenObject->onHostGame = [this]() {
        show(ScreenID::Host);
    };
    loginScreenObject->onPlayerGame = [this]() {
        show(ScreenID::Join);
    };

    hostScreenObject->onInvite = [this](const string& username, const string& password, int numPlayers) {
        if (onHostInvite) {
            onHostInvite(username, password, numPlayers);
        }
    };
    hostScreenObject->onLaunch = [this]() {
        if (onHostLaunch) {
            onHostLaunch();
        }
        show(ScreenID::Poker);
    };
    hostScreenObject->onLobby = [this]() {
        show(ScreenID::Login);
    };

    joinScreenObject->onConfirmedJoin = [this](const string& username, const string& password, int slot) {
        // ZZZ: NOT SURE YET
        if (onJoinConfirm){
            onJoinConfirm(username, password, slot);
        }
        show(ScreenID::Poker);
    };

    joinScreenObject->onLobby = [this]() {
        show(ScreenID::Login);
    };

    pokerScreenObject->onFold = [this]() {
        sendPlayerActionToServer(SocketFD, ACTION_FOLD, 0);
        if (onFold) {
            onFold();
        }
    };

    // ZZZ: CHECK THIS
    pokerScreenObject->onCheck = [this]() {
        sendPlayerActionToServer(SocketFD, ACTION_CHECK, 0);
        if (onCheck) {
            onCheck();
        }
    };

    pokerScreenObject->onBet = [this](int amount) {
        sendPlayerActionToServer(SocketFD, ACTION_BET, amount);
        if (onBet){
            onBet(amount);
        }
    };

    pokerScreenObject->onAllIn = [this]{
        sendPlayerActionToServer(SocketFD, ACTION_ALLIN, 0);

        if (onAllIn){
            onAllIn();
        }
    };

    gameOverScreenObject->onPlayAgain = [this]() {
        if (onPlayAgain) {
            onPlayAgain();
        }
        show(ScreenID::Host);
    };

    gameOverScreenObject->onExitToLobby = [this]() {
        if (onExitToLobby) {
            onExitToLobby();
        }
        show(ScreenID::Login);
    };
    */

    pokerScreenObject -> onFold = [this]() {
        int turn = officialGameState.playerTurn;
        if (turn >= 0 && turn < (int)officialGameState.players.size()) {
            officialGameState.players[turn].isInHand = 0; // player has folded
            officialGameState.players[turn].bet = Fold; // -1 from within data.hpp

        }
        ServerGameStateWrite(officialGameState);
        if (onFold){
            onFold();
        }
    };

    pokerScreenObject->onCheck = [this](){
        int turn = officialGameState.playerTurn;
        if(turn >= 0 && turn < (int)officialGameState.players.size()){
            officialGameState.players[turn].bet = Check; // 0 from within data.hpp

        }
        ServerGameStateWrite(officialGameState);
        if (onCheck) {
            onCheck();
        }
    };

    pokerScreenObject->onBet = [this](int amount) {
        int turn = officialGameState.playerTurn;
        if (turn >= 0 && turn < (int)officialGameState.players.size()){
            officialGameState.players[turn].bet = amount;
            if (officialGameState.players[turn].score >= amount) {
                officialGameState.players[turn].score -= amount;
                officialGameState.pot += amount;
            }
        }
        ServerGameStateWrite(officialGameState);
        if (onBet) {
            onBet(amount);
        }
    };

    pokerScreenObject->onAllIn = [this]{
        int turn = officialGameState.playerTurn;
        if(turn >= 0 && turn < (int)officialGameState.players.size()){
            int remainStack = officialGameState.players[turn].score;
            officialGameState.players[turn].bet += remainStack;
            officialGameState.pot += remainStack;
            officialGameState.players[turn].score = 0;
        }
        ServerGameStateWrite(officialGameState);
        if (onAllIn) {
            onAllIn();
        }
    };

    gameOverScreenObject->onPlayAgain = [this]() {
        if(onPlayAgain) {
            onPlayAgain();
        }
        show(ScreenID::Host);
    };

    gameOverScreenObject->onExitToLobby = [this]() {
        if(onExitToLobby) {
            onExitToLobby();
        }
        show(ScreenID::Login);
    };

} 

void ClientGUI::updateHostPlayerList(const vector<PLAYER>& players, int maxPlayers) {
    hostScreenObject->playerList(players, maxPlayers);
}

void ClientGUI::updateJoinPlayerList(const vector<PLAYER>& names, int maxPlayers) {
    joinScreenObject->playerList(names, maxPlayers);
}

void ClientGUI::setAvailableSlots(const vector<int>& slots) { 
    // changed to match joinScreen calling
    joinScreenObject->openSlots(slots);
}

void ClientGUI::updateGameState(const vector<PLAYER>& players, const vector<Card>& community,
                                 const vector<Card>& hole, int pot, int currentBet, int localStack,
                                 int playerTurn, int dealerTurn) {
    pokerScreenObject->updateGameState(players, community, hole, pot, currentBet, localStack, 
                                        playerTurn, dealerTurn);
}

void ClientGUI::setGameActions(bool enabled) { 
    pokerScreenObject->setActions(enabled);
}

void ClientGUI::setResults(const vector<PLAYER>& results, const GAMESTATE& summary) {
    gameOverScreenObject->setResults(results, summary);
    show(ScreenID::GameOver);
}

void ClientGUI::onWindowDestroy(GtkWidget*, gpointer) {
    gtk_main_quit();
}


/* MAIN FUNCTION */
int main(int argc, char *argv[]){
    int PortNo;
    struct hostent *Server;

    Program = argv[0];

    if (argc < 3){  

        char actualServer[100];
        printf("Before starting the login menu, please specify where the server actually is (e.g. an IP address; \'bondi\'; etc):  ");
        scanf(" %s", actualServer);

        Server = gethostbyname(actualServer);
        PortNo = DEFAULT_SERVERPORT;
        
    } else{ 
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


    /* Connecting to Server*/
    SocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (SocketFD < 0){   
        FatalError("socket creation failed");
    }
    if (connect(SocketFD, (struct sockaddr*)&ServerAddress,
            sizeof(struct sockaddr_in)) < 0){   
                FatalError("connecting to server failed");
    }

    /* LOGIN MENU 
    LOGININFO loginInfo;
    loginInfo = Talk2ServerLogin(loginInfo);
    MainMenu(&argc, &argv, loginInfo);
    */

    ClientGUI app(argc, argv);
    app.run();

    // Wrapup
    close(SocketFD);
    return 0;    
}
