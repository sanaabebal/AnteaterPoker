#include "clientGUI.hpp"
#include <stdexcept>

using namespace std;

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

    mainMenu = make_unique<loginScreen>();
    hostScreen = make_unique<HostScreen>();
    joinScreen = make_unique<JoinScreen>();
    gameScreen = make_unique<pokerScreen>();
    gameOverScreen = make_unique<GameOverScreen>();

    gtk_stack_add_named(GTK_STACK(stack), mainMenu->getWidget(), SCR_MAIN);
    gtk_stack_add_named(GTK_STACK(stack), hostScreen->getWidget(), SCR_HOST);
    gtk_stack_add_named(GTK_STACK(stack), joinScreen->getWidget(), SCR_JOIN);
    gtk_stack_add_named(GTK_STACK(stack), gameScreen->getWidget(), SCR_GAME);
    gtk_stack_add_named(GTK_STACK(stack), gameOverScreen->getWidget(), SCR_GAMEOVER);
}

void ClientGUI::wireCallbacks() {
    mainMenu->onHostGame = [this]() {
        show(ScreenID::Host);
    };
    mainMenu->onJoinGame = [this]() {
        show(ScreenID::Join);
    };

    hostScreen->onInvite = [this](const string& user,
                                   const string& pass,
                                   int numPlayers) {
        if (onHostInvite) {
            onHostInvite(user, pass, numPlayers);
        }
    };
    hostScreen->onLaunch = [this]() {
        if (onHostLaunch) {
            onHostLaunch();
        }
        show(ScreenID::Poker);
    };
    hostScreen->onLobby = [this]() {
        show(ScreenID::Login);
    };

    joinScreen->onConfirmJoin = [this](const string& user, const string& pass, int slot) {
        if (onJoinConfirm){
            onJoinConfirm(user, pass, slot);
        }
        show(ScreenID::Poker);
    };

    joinScreen->onLobby = [this]() {
        show(ScreenID::Login);
    };

    gameScreen->onFold  = [this]() { if (onFold)  onFold(); };
    gameScreen->onCheck = [this]() { if (onCheck) onCheck(); };
    gameScreen->onBet   = [this](int amount) { if (onBet) onBet(amount); };

    gameOverScreen->onPlayAgain = [this]() {
        if (onPlayAgain) {
            onPlayAgain();
        }
        show(ScreenID::Host);
    };
    gameOverScreen->onExitToLobby = [this]() {
        if (onExitToLobby) {
            onExitToLobby();
        }
        show(ScreenID::Login);
    };
}

void ClientGUI::updateHostPlayerList(const vector<RegisteredPlayer>& players, int maxPlayers) {
    hostScreen->updatePlayerList(players, maxPlayers);
}

void ClientGUI::updateJoinPlayerList(const vector<string>& names, const vector<int>& slots, int maxPlayers) {
    joinScreen->updatePlayerList(names, slots, maxPlayers);
}

void ClientGUI::setAvailableSlots(const vector<int>& slots) { 
    joinScreen->setAvailableSlots(slots);
}

void ClientGUI::updateGameState(const vector<PlayerInfo>& players, const vector<Card>& community,
                                 const vector<Card>& hole, int pot, int currentBet, int localStack) {
    gameScreen->updateGameState(players, community, hole, pot, currentBet, localStack);
}

void ClientGUI::setGameActions(bool enabled) { 
    gameScreen->setActions(enabled);
}

void ClientGUI::setResults(const vector<FinalPlayerResult>& results, const SessionSummary& summary) {
    gameOverScreen->setResults(results, summary);
    show(ScreenID::GameOver);
}

void ClientGUI::onWindowDestroy(GtkWidget*, gpointer) {
    gtk_main_quit();
}