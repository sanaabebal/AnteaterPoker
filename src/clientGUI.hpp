#ifndef CLIENTGUI
#define CLIENTGUI

#pragma once
#include <gtk/gtk.h>
#include <memory>

#include "cards.hpp"
#include "data.hpp"
#include "loginScreen.hpp"
#include "hostScreen.hpp"
#include "joinScreen.hpp"
#include "pokerScreen.hpp"
#include "gameOverScreen.hpp"

using namespace std;

enum class ScreenID {
    Login,
    Host,
    Join,
    Poker,
    GameOver
};

class ClientGUI {
public:
    ClientGUI(int argc, char** argv);
    ~ClientGUI();

    void run();

    void show(ScreenID id);

    void updateHostPlayerList(const vector<RegisteredPlayer>& players, int maxPlayers);
    void updateJoinPlayerList(const vector<string>& names,
                              const vector<int>& slots, int maxPlayers);
    void setAvailableSlots(const vector<int>& slots);
    void updateGameState(const vector<PlayerInfo>& players,
                         const vector<Card>& community,
                         const vector<Card>& hole,
                         int pot, int currentBet, int localStack);
    void setGameActions(bool enabled);
    void setResults(const vector<FinalPlayerResult>& results,
                            const SessionSummary& summary);


    function<void(const string& username,
                       const string& password,
                       int numPlayers)> onHostInvite;
    function<void()> onHostLaunch;
    function<void(const string& username,
                       const string& password,
                       int slot)> onJoinConfirm;
    function<void()> onFold;
    function<void()> onCheck;
    function<void(int amount)> onBet;
    function<void()> onPlayAgain;
    function<void()> onExitToLobby;

private:
    GtkWidget* window;
    GtkWidget* stack;   

    unique_ptr<loginScreen> mainMenu;
    unique_ptr<HostScreen> hostScreen;
    unique_ptr<JoinScreen> joinScreen;
    unique_ptr<PokerScreen> gameScreen;
    unique_ptr<GameOverScreen> gameOverScreen;

    void buildWindow(int argc, char** argv);
    void wireCallbacks();

    static const char* screenName(ScreenID id);

    static void onWindowDestroy(GtkWidget*, gpointer);
};

#endif