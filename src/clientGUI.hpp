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
                         const vector<Card>& hole, int pot, int currentBet, int localStack);

    void setGameActions(bool enabled);
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