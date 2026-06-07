#include <gtk/gtk.h>
#include "testScreen.hpp"

static void activate(
    GtkApplication* app,
    gpointer user_data
) {

    GtkWidget* window;

    window =
        gtk_application_window_new(app);

    gtk_window_set_title(
        GTK_WINDOW(window),
        "Poker Game"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(window),
        1000,
        700
    );

    // CREATE SCREEN
    GameOverScreen gameOver;

    // ADD SCREEN TO WINDOW
    gtk_container_add(
        GTK_CONTAINER(window),
        gameOver.getWidget()
    );

    gtk_widget_show_all(window);
}

int main(int argc, char** argv) {

    GtkApplication* app;

    int status;

    app = gtk_application_new(
        "com.poker.gui",
        G_APPLICATION_DEFAULT_FLAGS
    );

    g_signal_connect(
        app,
        "activate",
        G_CALLBACK(activate),
        NULL
    );

    status =
        g_application_run(
            G_APPLICATION(app),
            argc,
            argv
        );

    g_object_unref(app);

    return status;
}