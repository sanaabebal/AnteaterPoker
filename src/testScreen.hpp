#ifndef TEST_SCREEN_HPP
#define TEST_SCREEN_HPP

#include <gtk/gtk.h>

class GameOverScreen {
public:
    GameOverScreen();

    GtkWidget* getWidget();

private:
    GtkWidget* mainBox;

    GtkWidget* createPlayerRow(
        const char* rank,
        const char* playerName,
        const char* chipText
    );
};

#endif