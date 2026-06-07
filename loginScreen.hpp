// loginScreen.hpp

#pragma once
#include <gtk/gtk.h>
#include <functional>

using namespace std;

class loginScreen {
    public:
        loginScreen();
        ~loginScreen();

        GtkWidget* getWidget();

        function<void()> onHostGame;
        function<void()> onPlayerGame;

    private:
        GtkWidget *box;
        GtkWidget *titleLabel;
        GtkWidget *hostButton;
        GtkWidget *playerButton;

        void buildUI();

        void applyStyles();

        static void onHostClicked(GtkButton* button, gpointer inputData);
        static void onPlayerClicked(GtkButton* button, gpointer inputData);

};