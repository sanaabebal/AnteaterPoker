/* clientGUI.hpp */

#pragma once

#include <gtk/gtk.h>

#include "data.hpp"
#include "loginScreen.hpp"
#include "hostScreen.hpp"
#include "playerScreen.hpp"

class ClientGUI {
    public:
        ClientGUI(int argc, char** argv);

        int run();
    
    private:
        GtkApplication* app;
        GtkApplication* window;
        
        
        GtkWidget* stack;

        GtkWidget* playerCards;
        GtkWidget* communityCards;

        GtkWidget* callButton;
        GtkWidget* raiseButton;
        GtkWidget* foldButton;

        GtkWidget* raiseEntry;

        GAMESTATE officialGameState;
        LOGININFO playerLoginInfo;

        ClientNetworth* network;

        static void raiseClicked (
                GtkWidget* widget,
                gpointer data
        );

        static void foldClicked (
                GtkWidget* widget,
                gpointer data
        );

        void buildGUI();
        void updateTable();

        void clearContainers(
            GtkWidget* container
        );

        GtkWidget* createCard (
            const Card& card,
            bool visible
        );
};