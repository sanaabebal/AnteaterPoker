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
// CODEX FIX: tracks a joined client that registered successfully but is waiting for the host to launch.
static bool waitingForHostLaunch = false;

// CODEX FIX: use named lobby markers so host launch/status messages are not confused with real game rounds.
static const int LOBBY_ROSTER_REQUEST = -1;
// CODEX FIX: marker sent by the host when the Launch New Session button is clicked.
static const int LOBBY_START_REQUEST = -2;
// CODEX FIX: marker sent by joined clients while waiting for the host to launch.
static const int LOBBY_STATUS_REQUEST = -3;
// CODEX FIX: lobby-only status flag stored in GAMESTATE.callAmount while the game is still waiting.
static const int LOBBY_GAME_WAITING = 0;
// CODEX FIX: lobby-only status flag stored in GAMESTATE.callAmount once the host starts the game.
static const int LOBBY_GAME_STARTED = 1;

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

// CODEX FIX: request the latest server-owned login/room state without registering a player.
LOGININFO RefreshServerLoginInfo(){
    LOGININFO request;
    // CODEX FIX: playerNum -1 is the existing protocol's "status request" marker.
    request.playerNum = -1;
    return Talk2ServerLogin(request);
}

// CODEX FIX: build the open-slot list from the server-owned playersFound vector.
vector<int> AvailableSlotsFromLoginInfo(const LOGININFO& loginInfo){
    vector<int> slots;
    for(unsigned int i=0; i<loginInfo.playersFound.size(); i++){
        if(loginInfo.playersFound[i] == 0){
            slots.push_back((int)i);
        }
    }
    return slots;
}

// CODEX FIX: create a minimal player row for the host screen after the host registers.
PLAYERS HostPlayerList(const string& username){
    PLAYERS hostPlayers;
    PLAYER hostPlayer;
    hostPlayer.playerNum = 0;
    strncpy(hostPlayer.name, username.c_str(), sizeof(hostPlayer.name) - 1);
    // CODEX FIX: force termination after strncpy so the host row label is clean.
    hostPlayer.name[sizeof(hostPlayer.name) - 1] = '\0';
    hostPlayer.playerType = Human;
    hostPlayers.push_back(hostPlayer);
    return hostPlayers;
}

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

// CODEX FIX: send a lobby-only GAMESTATE request and return the server's reply.
GAMESTATE RequestLobbyGameState(int requestType){
    int n;
    // CODEX FIX: keep lobby sync in the existing game packet format.
    GAMESTATE request;
    // CODEX FIX: numPlayers carries the lobby request marker before real game flow exists.
    request.numPlayers = requestType;
    BUF RecvBuf(2000);
    BUF SendBuf = createBuffer(request);

    n = write(SocketFD, SendBuf.data(), SendBuf.size());
    if (n < 0){
        FatalError("writing lobby request to socket failed");
    }

    n = read(SocketFD, RecvBuf.data(), 2000);
    if (n < 0) {
        FatalError("reading lobby response from socket failed");
    }
    RecvBuf.resize(n);

    return parsingGameArguments(RecvBuf);
}

// CODEX FIX: ask the server for the current lobby roster without starting/advancing a game.
GAMESTATE RequestServerRoster(){
    return RequestLobbyGameState(LOBBY_ROSTER_REQUEST);
}

// CODEX FIX: convert the server roster into only the registered players the host should see.
PLAYERS RegisteredPlayersFromRoster(const GAMESTATE& roster){
    PLAYERS registeredPlayers;
    for(unsigned int i=0; i<roster.players.size(); i++){
        if(roster.players[i].playerNum >= 0){
            registeredPlayers.push_back(roster.players[i]);
        }
    }
    return registeredPlayers;
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
    // CODEX FIX: poll the server so the host lobby sees players who join from other clients.
    g_timeout_add(1000, ClientGUI::onHostLobbyRefresh, this);
    // CODEX FIX: poll the server so joined clients move to the table when the host launches.
    g_timeout_add(1000, ClientGUI::onJoinLaunchPoll, this);
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
        // CODEX FIX: leaving the waiting lobby cancels any joined-client launch polling.
        waitingForHostLaunch = false;
        // CODEX FIX: refresh room state before allowing this client to become the host.
        playerLoginInfo = RefreshServerLoginInfo();
        if(playerLoginInfo.isHost == 1 && playerLoginInfo.numPlayers == 0){
            // CODEX FIX: keep the host flag explicit before showing the host setup screen.
            playerLoginInfo.isHost = 1;
            show(ScreenID::Host);
        } else{
            // CODEX FIX: if a host already exists, route this client to Join with current open slots.
            playerLoginInfo.isHost = 0;
            setAvailableSlots(AvailableSlotsFromLoginInfo(playerLoginInfo));
            joinScreenObject->showError("A game already exists. Join an open slot.");
            show(ScreenID::Join);
        }
    };
    loginScreenObject->onPlayerGame = [this]() {
        // CODEX FIX: reset waiting state before starting a fresh join attempt.
        waitingForHostLaunch = false;
        // CODEX FIX: refresh the server room before showing join slots/password validation.
        playerLoginInfo = RefreshServerLoginInfo();
        // CODEX FIX: this path is always a non-host join attempt.
        playerLoginInfo.isHost = 0;
        setAvailableSlots(AvailableSlotsFromLoginInfo(playerLoginInfo));
        if(playerLoginInfo.numPlayers <= 0){
            // CODEX FIX: tell the player why no join slots are available yet.
            joinScreenObject->showError("No hosted game is available yet.");
        } else{
            // CODEX FIX: clear stale join errors once a hosted game is available.
            joinScreenObject->clearError();
        }
        show(ScreenID::Join);
    };

    hostScreenObject->onInvite = [this](const string& username, const string& password, int numPlayers) {
        // CODEX FIX: send host setup to the server; the previous GUI only changed local state.
        LOGININFO submittedLogin = playerLoginInfo;
        LOGININFO serverLogin = SendServerLogin(submittedLogin);
        // CODEX FIX: preserve local host identity while accepting the server-owned occupancy vector.
        playerLoginInfo = submittedLogin;
        playerLoginInfo.playersFound = serverLogin.playersFound;
        playerLoginInfo.numPlayers = serverLogin.numPlayers;
        // CODEX FIX: show the host as registered in the host screen immediately after server confirmation.
        updateHostPlayerList(HostPlayerList(username), numPlayers);
        if (onHostInvite) {
            onHostInvite(username, password, numPlayers);
        }
    };
    hostScreenObject->onLaunch = [this]() {
        // CODEX FIX: tell the server the host launched before showing Poker locally.
        GAMESTATE launchState = RequestLobbyGameState(LOBBY_START_REQUEST);
        if(launchState.callAmount != LOBBY_GAME_STARTED){
            // CODEX FIX: keep the host in setup if the server says the roster is not full yet.
            refreshHostLobby();
            return;
        }
        if (onHostLaunch) {
            onHostLaunch();
        }
        // CODEX FIX: show the same server-synced roster that waiting clients will receive.
        showSyncedPokerTable(launchState);
    };
    hostScreenObject->onLobby = [this]() {
        // CODEX FIX: leaving the host setup screen should not leave stale waiting state behind.
        waitingForHostLaunch = false;
        show(ScreenID::Login);
    };

    joinScreenObject->onConfirmedJoin = [this](const string& username, const string& password, int slot) {
        // CODEX FIX: refresh room state at click time so the slot/password check is not stale.
        LOGININFO serverLogin = RefreshServerLoginInfo();
        if(serverLogin.numPlayers <= 0){
            // CODEX FIX: keep the player on Join and explain that no host has registered.
            setAvailableSlots(AvailableSlotsFromLoginInfo(serverLogin));
            joinScreenObject->showError("No hosted game is available yet.");
            return;
        }
        if(strcmp(password.c_str(), serverLogin.password) != 0){
            // CODEX FIX: enforce the room password before sending a join request.
            setAvailableSlots(AvailableSlotsFromLoginInfo(serverLogin));
            joinScreenObject->showError("Incorrect room password.");
            return;
        }
        if(slot < 0 || slot >= (int)serverLogin.playersFound.size() || serverLogin.playersFound[slot] != 0){
            // CODEX FIX: reject invalid or newly occupied slots instead of sending a bad join.
            setAvailableSlots(AvailableSlotsFromLoginInfo(serverLogin));
            joinScreenObject->showError("That slot is no longer available.");
            return;
        }
        // CODEX FIX: build a server-valid join packet from the latest server-owned room state.
        LOGININFO submittedLogin = serverLogin;
        submittedLogin.isHost = 0;
        submittedLogin.playerNum = slot;
        submittedLogin.playerType = Human;
        strncpy(submittedLogin.playerName, username.c_str(), sizeof(submittedLogin.playerName) - 1);
        // CODEX FIX: force termination after strncpy so the server displays the joined player's name correctly.
        submittedLogin.playerName[sizeof(submittedLogin.playerName) - 1] = '\0';
        strncpy(submittedLogin.password, password.c_str(), sizeof(submittedLogin.password) - 1);
        // CODEX FIX: force termination after strncpy so the server receives the exact password.
        submittedLogin.password[sizeof(submittedLogin.password) - 1] = '\0';
        submittedLogin.playersFound[slot] = Human;
        // CODEX FIX: send the validated join to the server; the previous GUI never sent this packet.
        LOGININFO updatedLogin = SendServerLogin(submittedLogin);
        if(slot >= (int)updatedLogin.playersFound.size() || updatedLogin.playersFound[slot] == 0){
            // CODEX FIX: keep the player on Join if the server refused the join.
            setAvailableSlots(AvailableSlotsFromLoginInfo(updatedLogin));
            joinScreenObject->showError("Server rejected the join request.");
            return;
        }
        // CODEX FIX: save confirmed join state and clear the visible error before entering the game screen.
        playerLoginInfo = submittedLogin;
        playerLoginInfo.playersFound = updatedLogin.playersFound;
        // CODEX FIX: enable launch polling now that this client is registered with the server.
        waitingForHostLaunch = true;
        // CODEX FIX: keep the joined player on the join screen as a waiting state until the host launches.
        setAvailableSlots(AvailableSlotsFromLoginInfo(updatedLogin));
        // CODEX FIX: tell the joined player they are registered instead of jumping straight to the table.
        joinScreenObject->showError("Joined. Waiting for host to launch.");
        if (onJoinConfirm){
            onJoinConfirm(username, password, slot);
        }
    };

    joinScreenObject->onLobby = [this]() {
        // CODEX FIX: cancel host-launch polling when the joined player goes back to the lobby.
        waitingForHostLaunch = false;
        show(ScreenID::Login);
    };

    // pokerScreenObject->onFold = [this]() {
    //     sendPlayerActionToServer(SocketFD, ACTION_FOLD, 0);
    //     if (onFold) {
    //         onFold();
    //     }
    // };

    // // ZZZ: CHECK THIS
    // pokerScreenObject->onCheck = [this]() {
    //     sendPlayerActionToServer(SocketFD, ACTION_CHECK, 0);
    //     if (onCheck) {
    //         onCheck();
    //     }
    // };

    // pokerScreenObject->onBet = [this](int amount) {
    //     sendPlayerActionToServer(SocketFD, ACTION_BET, amount);
    //     if (onBet){
    //         onBet(amount);
    //     }
    // };

    // pokerScreenObject->onAllIn = [this]{
    //     sendPlayerActionToServer(SocketFD, ACTION_ALLIN, 0);

    //     if (onAllIn){
    //         onAllIn();
    //     }
    // };

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

// CODEX FIX: render the server-approved launch state on the poker table.
void ClientGUI::showSyncedPokerTable(const GAMESTATE& state) {
    // CODEX FIX: keep the shared game state aligned with the roster the server just returned.
    officialGameState = state;
    // CODEX FIX: clear the lobby-only status flag before the poker UI treats it as a bet amount.
    officialGameState.callAmount = 0;

    vector<Card> communityCards;
    vector<Card> holeCards;
    int localStack = START_POINTS;
    for(unsigned int i=0; i<officialGameState.players.size(); i++){
        if(officialGameState.players[i].playerNum == playerLoginInfo.playerNum){
            // CODEX FIX: use the local player's stack from the synced server roster.
            localStack = officialGameState.players[i].score;
            break;
        }
    }

    // CODEX FIX: show the registered seats immediately; real cards can be dealt by the later game loop.
    updateGameState(officialGameState.players, communityCards, holeCards, officialGameState.pot,
                    officialGameState.callAmount, localStack, officialGameState.playerTurn,
                    officialGameState.dealerPlayer);
    // CODEX FIX: keep betting controls disabled until the real turn/game-state loop is implemented.
    setGameActions(false);
    show(ScreenID::Poker);
}

// CODEX FIX: GTK timeout bridge that calls the host lobby refresh method.
gboolean ClientGUI::onHostLobbyRefresh(gpointer data) {
    auto* self = static_cast<ClientGUI*>(data);
    self->refreshHostLobby();
    return TRUE;
}

// CODEX FIX: GTK timeout bridge that polls launch status for joined waiting clients.
gboolean ClientGUI::onJoinLaunchPoll(gpointer data) {
    auto* self = static_cast<ClientGUI*>(data);
    self->pollJoinLaunch();
    return TRUE;
}

// CODEX FIX: refresh the host lobby player list while the host is waiting for players.
void ClientGUI::refreshHostLobby() {
    if(playerLoginInfo.isHost != 1 || playerLoginInfo.numPlayers <= 0){
        return;
    }
    // CODEX FIX: stop host lobby polling once the host has left the setup screen.
    const gchar* visibleScreen = gtk_stack_get_visible_child_name(GTK_STACK(stack));
    if(visibleScreen == NULL || strcmp(visibleScreen, SCR_HOST) != 0){
        return;
    }

    // CODEX FIX: pull the latest server-owned player roster so joins become visible to the host.
    GAMESTATE roster = RequestServerRoster();
    PLAYERS registeredPlayers = RegisteredPlayersFromRoster(roster);
    updateHostPlayerList(registeredPlayers, playerLoginInfo.numPlayers);
}

// CODEX FIX: move a joined client to Poker only after the server says the host launched.
void ClientGUI::pollJoinLaunch() {
    if(!waitingForHostLaunch || playerLoginInfo.isHost == 1 || playerLoginInfo.numPlayers <= 0){
        return;
    }

    // CODEX FIX: ask the server for launch status without mutating the lobby roster.
    GAMESTATE status = RequestLobbyGameState(LOBBY_STATUS_REQUEST);
    if(status.callAmount != LOBBY_GAME_STARTED){
        return;
    }

    // CODEX FIX: stop polling once this client consumes the host launch signal.
    waitingForHostLaunch = false;
    // CODEX FIX: use the server response that marked the game started to render Poker.
    showSyncedPokerTable(status);
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

    // CODEX FIX: fetch initial room state so host/join screens know whether a host already exists.
    playerLoginInfo = RefreshServerLoginInfo();

    ClientGUI app(argc, argv);
    app.run();

    // Wrapup
    close(SocketFD);
    return 0;    
}
