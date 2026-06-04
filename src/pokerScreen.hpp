// pokerScreen.hpp

#pragma once
#pragma once
#include <gtk/gtk.h>
#include <functional>
#include <string>
#include <vector>

using namespace std;

struct playerInfo {
    string name;
    string status;
    int stack;
    bool isDealer;
    bool yourTurn;
    int avatarIndex;
};

struct Card {
    string rank;
    string suit;
    bool faceDown;

};

class gameScreen {
    public:
        gameScreen();
        ~gameScreen();

        GtkWidget* getWidget();

        void 

    private:
}