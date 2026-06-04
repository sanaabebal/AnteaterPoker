// joinScreen.hpp
#pragma once
#include <gtk/gtk.h>
#include <functional>
#include <vector>
#include <string>

using namespace std;

struct RegisteredPlayer {
    string name;
    int slot;
};

class joinScreen {
    public:
        joinScreen();
        ~joinScreen();

        GtkWidget* getWidget();

        void playerList(const vector<RegisteredPlayer>& players,
                        int maxPlayers);

        void openSlots(const vector<int>& slots);

        function<void(const string& username,
                    const string& password,
                    int slot)> onConfirmedJoin;
        
        function<void()> onLobby;
    private:
        GtkWidget *container;
        GtkWidget *userEntry;
        GtkWidget *pwdEntry;
        GtkWidget *slotCombo;
        GtkWidget *listBox;
        GtkWidget *playerCountLabel;
        GtkWidget *confirmButton;
        GtkWidget *lobbyButton;

        void buildUI();
        void applyStyles();

        static void onConfirmedJoinClicked(GtkButton* button, gpointer inputData);
        static void lobbyClicked(GtkButton* button, gpointer inputData);


};