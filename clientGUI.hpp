#ifndef CLIENT_GUI_HPP
#define CLIENT_GUI_HPP

#pragma once
#include <gtk/gtk.h>
#include <vector>
#include <string>
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

    void updateHostPlayerList(const vector<PLAYER>& players, int maxPlayers);
    void updateJoinPlayerList(const vector<PLAYER>& players, int maxPlayers);
    void setAvailableSlots(const vector<int>& slots);
    void updateGameState(const vector<PLAYER>& players, const vector<Card>& community,
                         const vector<Card>& hole, int pot, int currentBet, int localStack,
                         int playerTurn, int dealerTurn);

    void setGameActions(bool enabled);
    // CODEX FIX: enable the next-hand button only when this client is the dealer between hands.
    void setStartHandAction(bool enabled);
    void setResults(const vector<PLAYER>& results, const GAMESTATE& summary);


    function<void(const string& username, const string& password, int numPlayers)> onHostInvite;
    function<void()> onHostLaunch;
    function<void(const string& username,const string& password, int slot)> onJoinConfirm;
    function<void()> onFold;
    function<void()> onCheck;
    function<void(int amount)> onBet;
    function<void()> onAllIn;
    function<void()> onPlayAgain;
    function<void()> onExitToLobby;

private:

    void buildWindow(int argc, char** argv);
    void wireCallbacks();
    // CODEX FIX: refresh the host lobby list from the server while waiting for players.
    void refreshHostLobby();
    // CODEX FIX: poll for the host launch while a joined player is waiting in the lobby.
    void pollJoinLaunch();
    // CODEX FIX: render the server-synced lobby roster on the poker table after launch.
    void showSyncedPokerTable(const GAMESTATE& state);
    // CODEX FIX: refresh the visible poker table from the server while the game is running.
    void refreshRunningGame();
    // CODEX FIX: GTK timeout bridge for host lobby polling.
    static gboolean onHostLobbyRefresh(gpointer data);
    // CODEX FIX: GTK timeout bridge for joined-player launch polling.
    static gboolean onJoinLaunchPoll(gpointer data);
    // CODEX FIX: GTK timeout bridge for running-game state polling.
    static gboolean onGameStatePoll(gpointer data);

    static const char* screenName(ScreenID id);

    static void onWindowDestroy(GtkWidget*, gpointer);

    GtkWidget* window;
    GtkWidget* stack;   

    unique_ptr<loginScreen> loginScreenObject;
    unique_ptr<hostScreen> hostScreenObject;
    unique_ptr<joinScreen> joinScreenObject;
    unique_ptr<pokerScreen> pokerScreenObject;
    unique_ptr<gameOverScreen> gameOverScreenObject;

};

#endif
