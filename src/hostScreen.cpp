#include "hostScreen.hpp"
#include "data.hpp"

#include <string>
#include <sstream>
using namespace std;

extern LOGININFO playerLoginInfo;

static const char* hostCSS = R"CSS(
#hs-root {
    background-color: #2b4c54;
}
#hs-dialog {
    background-color: #e8d9a0;
    border: 3px solid #8b7340;
    border-radius: 6px;
    padding: 20px;
}
.hs-label {
    color: #3a2a0e;
    font-family: "Georgia", serif;
    font-size: 12px;
}
.hs-title {
    color: #2c1f0e;
    font-family: "Georgia", serif;
    font-size: 18px;
    font-weight: bold;
    letter-spacing: 2px;
}
.hs-entry {
    background-color: #f5ecc8;
    border: 2px solid #a08040;
    color: #2c1f0e;
    font-family: "Georgia", serif;
    font-size: 13px;
    padding: 4px 6px;
    border-radius: 3px;
}
.hs-section-label {
    color: #3a2a0e;
    font-family: "Georgia", serif;
    font-size: 11px;
    font-weight: bold;
}
.hs-player-row {
    color: #2c1f0e;
    font-family: "Georgia", serif;
    font-size: 12px;
    padding: 2px 4px;
}
.hs-combo {
    background-color: #b8860b;
    color: #f5ecc8;
    font-family: "Georgia", serif;
    font-size: 13px;
    border-radius: 3px;
    border: 1px solid #6b5010;
    padding: 4px;
}
.hs-player-list {
    background-color: #f5ecc8;
    border: 1px solid #8b7340;
    border-radius: 4px;
}
.hs-invite {
    background-color: #b8860b;
    color: #f5ecc8;
    font-family: "Georgia", serif;
    font-size: 13px;
    font-weight: bold;
    border-radius: 4px;
    border: 2px solid #a08040;
    padding: 8px 12px;
}
.hs-invite:hover {
    background-color: #d4a010;
}
.hs-launch {
    background-color: #888;
    color: #ccc;
    font-family: "Georgia", serif;
    font-size: 13px;
    border-radius: 4px;
    border: 2px solid #666;
    padding: 8px 12px;
}
#hs-launch:active {
    background-color: #4a7a4a;
    color: #f5ecc8;
    border-color: #2a5a2a;
}
.hs-lobby {
    background-color: #e8d9a0;
    color: #2c1f0e;
    font-family: "Georgia", serif;
    font-size: 12px;
    border: 2px solid #a08040;
    border-radius: 4px;
    padding: 4px 24px;
}
.hs-lobby:hover {
    background-color: #f0e6b8;
}
)CSS";

hostScreen::hostScreen() {
    buildUI();
    applyStyles();
}

hostScreen::~hostScreen() {}

GtkWidget* hostScreen::getWidget() {
    return container; 
}

void hostScreen::buildUI() {
    container = gtk_overlay_new();
    gtk_widget_set_name(container, "hs-root");

    // chose vertical over horizontal to get rid of the column spacing and division of space
    GtkWidget* background = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(background, "hs-root");
    gtk_widget_set_hexpand(background, TRUE);
    gtk_widget_set_vexpand(background, TRUE);
    gtk_container_add(GTK_CONTAINER(container), background);

    /*
    GtkWidget* top = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(background), top, FALSE, FALSE, 4);

    GtkWidget* titleLabel = gtk_label_new("ANTEATER POKER");
    gtk_widget_set_name(titleLabel, "hs-title");
    gtk_widget_set_halign(titleLabel, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(top), titleLabel, TRUE, TRUE, 0);
    */

    GtkWidget *dialogFrame = gtk_frame_new(nullptr);
    gtk_widget_set_name(dialogFrame, "hs-dialog");
    gtk_widget_set_halign(dialogFrame, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(dialogFrame, GTK_ALIGN_CENTER);
    gtk_widget_set_vexpand(dialogFrame, TRUE);
    gtk_box_pack_start(GTK_BOX(background), dialogFrame, TRUE, TRUE, 20);

    GtkWidget *dialogBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_set_margin_top(dialogBox, 16);
    gtk_widget_set_margin_bottom(dialogBox, 16);
    gtk_widget_set_margin_start(dialogBox, 20);
    gtk_widget_set_margin_end(dialogBox, 20);
    gtk_container_add(GTK_CONTAINER(dialogFrame), dialogBox);

    GtkWidget *left = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_box_pack_start(GTK_BOX(dialogBox), left, FALSE, FALSE, 0);

    GtkWidget *dialogTitle = gtk_label_new("SETTING UP A NEW GAME");
    gtk_widget_set_name(dialogTitle, "hs-title");
    gtk_box_pack_start(GTK_BOX(left), dialogTitle, FALSE, FALSE, 8);

    // username
    GtkWidget *userLabel = gtk_label_new("Player Username");
    gtk_widget_set_name(userLabel, "hs-label");
    gtk_widget_set_halign(userLabel, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(left), userLabel, FALSE, FALSE, 0);

    userEntry = gtk_entry_new();
    gtk_widget_set_name(userEntry, "hs-entry");
    gtk_entry_set_placeholder_text(GTK_ENTRY(userEntry), "Your name");
    gtk_widget_set_size_request(userEntry, 200, -1);
    gtk_box_pack_start(GTK_BOX(left), userEntry, FALSE, FALSE, 0);

    // password
    GtkWidget *passLabel = gtk_label_new("Session Password");
    gtk_widget_set_name(passLabel, "hs-label");
    gtk_widget_set_halign(passLabel, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(left), passLabel, FALSE, FALSE, 0);

    pwdEntry = gtk_entry_new();
    gtk_widget_set_name(pwdEntry, "hs-entry");
    gtk_entry_set_placeholder_text(GTK_ENTRY(pwdEntry), "Password");
    gtk_widget_set_size_request(pwdEntry, 200, -1);
    gtk_box_pack_start(GTK_BOX(left), pwdEntry, FALSE, FALSE, 0);

    // number of players
    GtkWidget *number = gtk_label_new("Number of Players");
    gtk_widget_set_name(number, "hs-label");
    gtk_widget_set_halign(number, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(left), number, FALSE, FALSE, 0);

    numCombo = gtk_combo_box_text_new();
    gtk_widget_set_name(numCombo, "hs-combo");
    for (int i = 2; i <= 10; ++i) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(numCombo),
                                        to_string(i).c_str());
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(numCombo), 2);
    gtk_widget_set_size_request(numCombo, 200, -1);
    gtk_box_pack_start(GTK_BOX(left), numCombo, FALSE, FALSE, 0);

    GtkWidget *spacing = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_vexpand(spacing, TRUE);
    gtk_box_pack_start(GTK_BOX(left), spacing, TRUE, TRUE, 0);


    inviteButton = gtk_button_new_with_label ("Waiting for Players");

    gtk_widget_set_name(inviteButton, "hs-invite");
    gtk_widget_set_size_request(inviteButton, 200, 52);
    gtk_box_pack_start(GTK_BOX(left), inviteButton, FALSE, FALSE, 0);
    g_signal_connect(inviteButton, "clicked", G_CALLBACK(inviteClicked), this);

    // for registerd players
    GtkWidget *right = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_size_request(right, 220, -1);
    gtk_box_pack_start(GTK_BOX(dialogBox), right, FALSE, FALSE, 0);

    playerCountLabel = gtk_label_new("REGISTERED PLAYERS");
    gtk_widget_set_name(playerCountLabel, "hs-section-label");
    gtk_widget_set_halign(playerCountLabel, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(right), playerCountLabel, FALSE, FALSE, 0);

    GtkWidget *makeHeader = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(right), makeHeader, FALSE, FALSE, 0);

    auto header = [](const char *text, int w) -> GtkWidget* {
        GtkWidget *sectionLabel = gtk_label_new(text);
        gtk_widget_set_name(sectionLabel, "hs-section-label");
        gtk_widget_set_size_request(sectionLabel, w, -1);
        gtk_widget_set_halign(sectionLabel, GTK_ALIGN_START);
        return sectionLabel;
    };

    // container variable is the makeheader while the lambda name is header
    gtk_box_pack_start(GTK_BOX(makeHeader), header("Player", 100), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(makeHeader), header("Slot", 60), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(makeHeader), header("Join", 60), FALSE, FALSE, 0);

    // scrollable player lists
    GtkWidget *scroll = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scroll, 220, 150);
    gtk_box_pack_start(GTK_BOX(right), scroll, TRUE, TRUE, 0);

    listBox = gtk_list_box_new();
    gtk_widget_set_name(listBox, "hs-player-list");
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(listBox), GTK_SELECTION_NONE);
    gtk_container_add(GTK_CONTAINER(scroll), listBox);

    // spacing and then launch button
    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_vexpand(spacer, TRUE);
    gtk_box_pack_start(GTK_BOX(right), spacer, TRUE, TRUE, 0);

    launchButton = gtk_button_new_with_label("Launch New Session");
    gtk_widget_set_name(launchButton, "hs-launch");
    gtk_widget_set_size_request(launchButton, 200, 42);
    gtk_widget_set_sensitive(launchButton, FALSE);
    gtk_box_pack_start(GTK_BOX(right), launchButton, FALSE, FALSE, 0);
    g_signal_connect(launchButton, "clicked", G_CALLBACK(launchClicked), this);

    GtkWidget *bottomBar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_bottom(bottomBar, 15);
    gtk_box_pack_start(GTK_BOX(background), bottomBar, FALSE, FALSE, 8);

    lobbyButton = gtk_button_new_with_label("Lobby");
    gtk_widget_set_name(lobbyButton, "hs-lobby");
    gtk_widget_set_size_request(lobbyButton, 200,50);
    gtk_widget_set_halign(lobbyButton, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(bottomBar), lobbyButton, FALSE, FALSE, 0);
    g_signal_connect(lobbyButton, "clicked", G_CALLBACK(lobbyClicked), this);
    
}

void hostScreen::applyStyles() {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, hostCSS, -1, nullptr);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);
}

void hostScreen::playerList(const vector<PLAYER>& players, int mp) {
    maxPlayers = mp;

    GList *rows = gtk_container_get_children(GTK_CONTAINER(listBox));
    for (GList* i = rows; i; i = i -> next) {
        gtk_widget_destroy(GTK_WIDGET(i -> data));
    }
    g_list_free(rows);

    for (auto& p : players) {
        GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_name(row, "hs-player-row");

        auto cell = [](const string& text, int w) -> GtkWidget* {
            GtkWidget *l = gtk_label_new(text.c_str());
            gtk_widget_set_size_request(l, w, -1);
            gtk_widget_set_halign(l, GTK_ALIGN_START);
            return l;
        };

        gtk_box_pack_start(GTK_BOX(row), cell(p.name, 100), FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(row), cell(to_string(p.playerNum), 60), FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(row), cell("✓", 60), FALSE, FALSE, 0);

        GtkWidget *listRow = gtk_list_box_row_new();
        gtk_container_add(GTK_CONTAINER(listRow), row);
        gtk_list_box_insert(GTK_LIST_BOX(listBox), listRow, -1);
    }

    gtk_widget_show_all(listBox);

    gtk_widget_set_sensitive(launchButton, TRUE);

}

void hostScreen::inviteClicked(GtkButton*, gpointer inputData) {
    auto *self = static_cast<hostScreen*>(inputData);
    
    if(!self -> onInvite) return;

    string user = gtk_entry_get_text(GTK_ENTRY(self -> userEntry));
    string pass = gtk_entry_get_text(GTK_ENTRY(self -> pwdEntry));
    int index = gtk_combo_box_get_active(GTK_COMBO_BOX(self -> numCombo));
    int num = index + 2;

    strncpy(playerLoginInfo.playerName, user.c_str(), sizeof(playerLoginInfo.playerName) - 1);
    strncpy(playerLoginInfo.password, pass.c_str(), sizeof(playerLoginInfo.password) - 1);
    playerLoginInfo.numPlayers = num;
    playerLoginInfo.playerNum = 0; 
    playerLoginInfo.playerType = Human;
    playerLoginInfo.playersFound.resize(num);
    playerLoginInfo.playersFound[0] = 1;
    
    if (self -> onInvite){
        self -> onInvite(user, pass, num);
    }
}

void hostScreen::launchClicked(GtkButton*, gpointer inputData) {
    auto *self = static_cast<hostScreen*>(inputData);

    if (self -> onLaunch) {
        self -> onLaunch();
    }
}

void hostScreen::lobbyClicked(GtkButton*, gpointer inputData) {
    auto *self = static_cast<hostScreen*>(inputData);


    if (self -> onLobby) {
        self -> onLobby();
    }
}

#ifdef TEST_HOST
int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Tester Host");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    hostScreen host;
    
    vector<RegisteredPlayer> mockPlayers = {{"Alice", 1}, {"Bob", 2}};
    host.playerList(mockPlayers, 4);

    gtk_container_add(GTK_CONTAINER(window), host.getWidget());

    gtk_widget_show_all(window);
    gtk_main();
    
    return 0;
}
#endif