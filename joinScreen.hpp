// joinScreen.hpp
#pragma once
#include <gtk/gtk.h>
#include <functional>
#include <vector>
#include <string>
#include "data.hpp"
#include "cards.hpp"

using namespace std;


class joinScreen {
    public:
        joinScreen();
        ~joinScreen();

        GtkWidget* getWidget();

        void playerList(const vector<PLAYER>& players, int maxPlayers);

        void openSlots(const vector<int>& slots);
        // CODEX FIX: expose a visible join-screen message for password/slot/server validation errors.
        void showError(const string& message);
        // CODEX FIX: expose a way to clear stale join-screen validation errors after a successful refresh.
        void clearError();

        function<void(const string& username, const string& password, int slot)> onConfirmedJoin;
        
        function<void()> onLobby;
    private:
        GtkWidget *container;
        GtkWidget *userEntry;
        GtkWidget *pwdEntry;
        GtkWidget *slotCombo;
        GtkWidget *listBox;
        GtkWidget *playerCountLabel;
        // CODEX FIX: store a visible error label so failed joins do not look like ignored clicks.
        GtkWidget *errorLabel;
        GtkWidget *confirmButton;
        GtkWidget *lobbyButton;

        void buildUI();
        void applyStyles();

        static void onConfirmedJoinClicked(GtkButton* button, gpointer inputData);
        static void lobbyClicked(GtkButton* button, gpointer inputData);


};
