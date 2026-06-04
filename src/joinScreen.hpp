// joinScreen.hpp
#pragma once
#include <gtk/gtk.h>
#include <functional>

using namespace std;

class JoinScreen {
    public:
        joinScreen();
        ~joinScreen()

        GtkWidget* getWidget();

        void playerList(const vector<RegisteredPlayer>& players,
                        const vector<int>& slots,
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
        GtkWidget *slot;
        GtkWidget *playerList;
        GtkWidget *playerCountLabel;
        GtkWidget *confirmButton;
        GtkWidget *lobbyButton;

        void buildUI();
        void applyStyles();

        static void onConfirmedJoin(GtkButton*, gpointer);
        static void lobbyClicked(GtkButton*, gpointer);


}