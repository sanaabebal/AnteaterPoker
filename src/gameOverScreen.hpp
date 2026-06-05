#ifndef GAMEOVER_SCREEN_HPP
#define GAMEOVER_SCREEN_HPP

#include <gtk/gtk.h>
#include <vector>
#include <string>
#include <utility>
#include <functional>

struct FinalPlayerResults {
    int rank;
    std::string username;
    int chips;
    int chipDelta;
    bool isWinner;
    std::vector<std::pair<std::string, std::string>> handCards;
};

struct gameSummary {
    int totalHands;
    std::string duration;
    int biggestPot;
};

class gameOverScreen {
public:
    gameOverScreen();
    ~gameOverScreen();

    GtkWidget* getWidget();
    void setResults(const std::vector<FinalPlayerResults>& results, const gameSummary& summary);

    // Public engine hook variables
    std::function<void()> onPlayAgain;
    std::function<void()> onExitToLobby;

private:
    void buildUI();
    void applyStyles();
    
    void populateResults(const std::vector<FinalPlayerResults>& results);
    void populateSummary(const gameSummary& summary);
    void drawnHandCards(cairo_t* cr, int w, int h);

    GtkWidget* container;
    GtkWidget* resultBox;
    GtkWidget* winnerHandArea; 
    GtkWidget* summaryBox;
    GtkWidget* playAgainButton;
    GtkWidget* exitLobbyButton;

    std::vector<std::pair<std::string, std::string>> winnerCards;

    // ── RENAMED STATIC GTK BRIDGE HANDLERS ──
    static gboolean onDrawHand(GtkWidget* widget, cairo_t* cr, gpointer data);
    static void onPlayAgainClicked(GtkButton* button, gpointer data);
    static void onExitLobbyClicked(GtkButton* button, gpointer data);
};

#endif // GAMEOVER_SCREEN_HPP