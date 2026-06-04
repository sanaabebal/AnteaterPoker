#include <gtk/gtk.h>
#include <string.h>

#include "loginScreen.hpp"

using namespace std;

static const char* loginCSS = R"CSS(
.ls-root {
    background-color: #2b4c54;
}
.ls-title {
    color: #d4b96a;
    font-family: "Georgia", serif;
    font-size: 32px;
    font-weight: bold;
    letter-spacing: 4px;
    text-shadow: 2px 2px 4px rgba(0,0,0,0.5);
}
.ls-button {
    background-color: #e8d9a0;
    color: #2c1f0e;
    font-family: "Georgia", serif;
    font-size: 16px;
    border-radius: 4px;
    border: 2px solid #a08040;
    padding: 8px 40px;
    min-width: 220px;
}
.ls-button:hover {
    background-color: #f0e6b8;
    border-color: #c0a050;
}
.ls-button:active {
    background-color: #c8b870;
}
)CSS";

loginScreen::loginScreen() {
    buildUI();
    applyStyles();
}

loginScreen::~loginScreen() {

}

GtkWidget* loginScreen::getWidget() {
    return box;
}

void loginScreen::buildUI() {

    // basic box area
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_name(box, "ls-root");
    gtk_widget_set_halign(box, GTK_ALIGN_FILL);
    gtk_widget_set_valign(box, GTK_ALIGN_FILL);
    gtk_widget_set_vexpand(box, TRUE);
    gtk_widget_set_hexpand(box, TRUE);

    // centering it vertically
    GtkWidget* top = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    gtk_widget_set_vexpand(top, TRUE);
    gtk_box_pack_start(GTK_BOX(box), top, TRUE, TRUE, 0);

    titleLabel = gtk_label_new("ANTEATER POKER");
    gtk_widget_set_name(titleLabel, "ls-title");
    gtk_box_pack_start(GTK_BOX(box), titleLabel, FALSE, FALSE, 0);

    GtkWidget *spacing = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    gtk_widget_set_size_request(spacing, -1, 24);
    gtk_box_pack_start(GTK_BOX(box), spacing, FALSE, FALSE, 0);

    hostButton = gtk_button_new_with_label("Create a Game (Host)");
    gtk_widget_set_name(hostButton, "ls-button");
    gtk_widget_set_halign(hostButton, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(box), hostButton, FALSE, FALSE, 0);
    g_signal_connect(hostButton, "clicked", G_CALLBACK(onHostClicked), this);

    playerButton = gtk_button_new_with_label("Join a Game");
    gtk_widget_set_name(playerButton, "ls-button");
    gtk_widget_set_halign(playerButton, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(box), playerButton, FALSE, FALSE, 0);
    g_signal_connect(playerButton, "clicked", G_CALLBACK(onPlayerClicked), this);

    GtkWidget* bottom = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    gtk_widget_set_vexpand(bottom, TRUE);
    gtk_box_pack_start(GTK_BOX(box), bottom, TRUE, TRUE, 0);

}

void loginScreen::applyStyles() {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, loginCSS, -1, nullptr);
    gtk_style_context_add_provider_for_screen (
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_object_unref(provider);

    GtkStyleContext *context;

    context = gtk_widget_get_style_context(box);
    gtk_style_context_add_class(context, "ls-root");

    context = gtk_widget_get_style_context(titleLabel);
    gtk_style_context_add_class(context, "ls-title");

    context = gtk_widget_get_style_context(hostButton);
    gtk_style_context_add_class(context, "ls-button");

    context = gtk_widget_get_style_context(playerButton);
    gtk_style_context_add_class(context, "ls-button");
}

void loginScreen::onHostClicked(GtkButton* , gpointer inputData){
    auto *self = static_cast<loginScreen*>(inputData);
    if (self -> onHostGame){
        self -> onHostGame();
    }
}

void loginScreen::onPlayerClicked(GtkButton* , gpointer inputData){
    auto *self = static_cast<loginScreen*>(inputData);
    if (self -> onPlayerGame){
        self -> onPlayerGame();
    }
}

#ifdef TEST_LOGIN
int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
    gtk_window_set_title(GTK_WINDOW(window), "Tester");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    loginScreen login;

    gtk_container_add(GTK_CONTAINER(window), login.getWidget());

    gtk_widget_show_all(window);
    gtk_main();
    
    return 0;
}
#endif