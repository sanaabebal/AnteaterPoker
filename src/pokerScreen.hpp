// pokerScreen.hpp

#pragma once
#include <gtk/gtk.h>
#include <functional>
#include <string>
#include <vector>
#include "data.hpp"
#include "cards.hpp"

using namespace std;

struct playerInfo {
    string name;
    string status;
    int stack;
    bool isDealer;
    bool yourTurn;
    int avatarIndex;
};

class pokerScreen {
    public:
        pokerScreen();
        ~pokerScreen();

        GtkWidget* getWidget();

        void updateGameState( const vector<PLAYER>& players,
                              const vector<Card>& communityCards,
                              const vector<Card>& holeCards,
                              int pot,
                              int currentBet,
                              int currentPlayerStack,
                              int playerTurn,
                              int dealerTurn
                            );
        
        void setActions(bool enable);

        function<void()> onFold;
        function<void()> onCheck;
        function<void(int)> onBet;
        function<void()> onAllIn;

    private:

        GtkWidget *container;
        GtkWidget *potLabel;
        GtkWidget *foldButton;
        GtkWidget *checkButton;
        GtkWidget *betButton;
        GtkWidget *betSpinButton;
        GtkWidget *allInButton;
        GtkWidget *tableDrawingArea;

        vector<playerInfo> cachedPlayers;
        vector<Card> cachedCommunity;
        vector<Card> cachedHole;

        int cachedPot = 0;
        int cachedBet = 50;
        int localStack = 0;

        void buildUI();
        void applyStyles();

        static gboolean onDraw(GtkWidget* widget, cairo_t* cr, gpointer data);
        void drawTable(cairo_t* cr, int w, int h);
        void drawCard(cairo_t* cr, double x, double y, double cardW,
                      double cardH, const Card& card);
        void drawPlayer(cairo_t* cr, int w, int h, 
                        const playerInfo& p, int seatIndex, int numSeats);

        static void onFoldClicked(GtkButton*, gpointer);
        static void onCheckClicked(GtkButton*, gpointer);
        static void onBetClicked(GtkButton*, gpointer);
        static void onAllInClicked(GtkButton*, gpointer);
};