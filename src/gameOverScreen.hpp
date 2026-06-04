// gameOverScreen.hpp

#pragma once
#include <gtk/gtk.h>
#include <functional>
#include <string>
#include <vector>

using namespace std;

struct FinalPlayerResults {
    int rank;
    string username;
    int chips;
    int chipDelta;

    bool isWinner;
    vector<pair<string, string>> handCards;

};

struct gameSummary {
    int totalHands;
    string duration;
    int biggestPot;
};

class gameOverScreen {
    public:
        gameOverScreen();
        ~gameOverScreen();

        GtkWidget* getWidget();

        void setResults(const vector<FinalPlayerResults>& results,
                        const gameSummary& summary );

        function<void()> onPlayAgain();
        function<void()> onExit;

    private:
        
        GtkWidget *container;
        GtkWidget *resultBox;
        GtkWidget *summaryBox;
        GtkWidget *playAgainButton;
        GtkWidget *exitLobbyButton;

        void buildUI();
        void applyStyles();

        void populateResults(const vector<FinalPlayerResults>& results);
        void populateSummary(const gameSummary& summary);

        static gboolean onDrawHand(GtkWidget* widget, cairo_t* cr, gpointer inputData);
        void drawnHandCards(cairo_t* cr, int w, int h);

        static void onPlayAgainButton(GtkButton*, gpointer);
        static void onExitLobbu(GtkButton*, gpointer);
};