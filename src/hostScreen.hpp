// hostScreen.hpp
#pragma once

#include <gtk/gtk.h>
#include <vector>
#include <string>
#include <functional>
using namespace std;

struct RegisteredPlayer {
    string name;
    int slot;
};

class hostScreen {
    public:
        hostScreen();
        ~hostScreen();

        GtkWidget *getWidget();

        void playerList(const vector<RegisteredPlayer>& players, int maxPlayers);

        function<void(const string& username, const string& password, int numPlayers)> onInvite;

        function<void()> onLaunch;
        function<void()> onLobby;


    private:

        GtkWidget *container;
        GtkWidget *userEntry;
        GtkWidget *pwdEntry;
        GtkWidget *numCombo;
        GtkWidget *listBox;
        GtkWidget *playerCountLabel;
        GtkWidget *inviteButton;
        GtkWidget *launchButton;
        GtkWidget *lobbyButton;

        int maxPlayers = 10;

        void buildUI();
        void applyStyles();

        static void inviteClicked(GtkButton*, gpointer);
        static void launchClicked(GtkButton*, gpointer);
        static void lobbyClicked(GtkButton*, gpointer);
};