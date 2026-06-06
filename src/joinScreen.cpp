// joinScreen.cpp
#include "joinScreen.hpp"
#include <string>
#include <sstream>

using namespace std;

static const char* joinCSS = R"CSS(
#js-root {
    background-color: #2b4c54;
}
#js-dialog {
    background-color: #e8d9a0;
    border: 3px solid #8b7340;
    border-radius: 6px;
    padding: 20px;
}
.js-label {
    color: #3a2a0e;
    font-family: "Georgia", serif;
    font-size: 12px;
}
.js-title {
    color: #2c1f0e;
    font-family: "Georgia", serif;
    font-size: 18px;
    font-weight: bold;
    letter-spacing: 2px;
}
.js-entry {
    background-color: #f5ecc8;
    border: 2px solid #a08040;
    color: #2c1f0e;
    font-family: "Georgia", serif;
    font-size: 13px;
    padding: 4px 6px;
    border-radius: 3px;
}
.js-section-label {
    color: #3a2a0e;
    font-family: "Georgia", serif;
    font-size: 11px;
    font-weight: bold;
}
.js-player-row {
    color: #2c1f0e;
    font-family: "Georgia", serif;
    font-size: 12px;
    padding: 2px 4px;
}
.js-combo {
    background-color: #b8860b;
    color: #f5ecc8;
    font-family: "Georgia", serif;
    font-size: 13px;
    border-radius: 3px;
    border: 1px solid #6b5010;
    padding: 4px;
}
.js-player-list {
    background-color: #f5ecc8;
    border: 1px solid #8b7340;
    border-radius: 4px;
}
.js-confirm {
    background-color: #b8860b;
    color: #f5ecc8;
    font-family: "Georgia", serif;
    font-size: 13px;
    font-weight: bold;
    border-radius: 4px;
    border: 2px solid #a08040;
    padding: 8px 12px;
}
.js-confirm:hover {
    background-color: #d4a010;
}
.js-lobby {
    background-color: #e8d9a0;
    color: #2c1f0e;
    font-family: "Georgia", serif;
    font-size: 12px;
    border: 2px solid #a08040;
    border-radius: 4px;
    padding: 4px 24px;
}
.js-lobby:hover {
    background-color: #f0e6b8;
}
)CSS";

joinScreen::joinScreen() {
    buildUI();
    applyStyles();
}

joinScreen::~joinScreen() {}

GtkWidget* joinScreen::getWidget() {
    return container; 
}

void joinScreen::buildUI() {
    container = gtk_overlay_new();
    gtk_widget_set_name(container, "js-root");

    GtkWidget* background = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(background, "js-root");
    gtk_widget_set_hexpand(background, TRUE);
    gtk_widget_set_vexpand(background, TRUE);
    gtk_container_add(GTK_CONTAINER(container), background);

    GtkWidget *dialogFrame = gtk_frame_new(nullptr);
    gtk_widget_set_name(dialogFrame, "js-dialog");
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

    GtkWidget *dialogTitle = gtk_label_new("JOINING A GAME");
    gtk_widget_set_name(dialogTitle, "js-title");
    gtk_box_pack_start(GTK_BOX(left), dialogTitle, FALSE, FALSE, 8);

    // Username
    GtkWidget *userLabel = gtk_label_new("Player Username");
    gtk_widget_set_name(userLabel, "js-label");
    gtk_widget_set_halign(userLabel, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(left), userLabel, FALSE, FALSE, 0);

    userEntry = gtk_entry_new();
    gtk_widget_set_name(userEntry, "js-entry");
    gtk_entry_set_placeholder_text(GTK_ENTRY(userEntry), "Your name");
    gtk_widget_set_size_request(userEntry, 200, -1);
    gtk_box_pack_start(GTK_BOX(left), userEntry, FALSE, FALSE, 0);

    // Password 
    GtkWidget *passLabel = gtk_label_new("Session Password");
    gtk_widget_set_name(passLabel, "js-label");
    gtk_widget_set_halign(passLabel, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(left), passLabel, FALSE, FALSE, 0);

    pwdEntry = gtk_entry_new();
    gtk_widget_set_name(pwdEntry, "js-entry");
    gtk_entry_set_placeholder_text(GTK_ENTRY(pwdEntry), "Password");
    gtk_widget_set_size_request(pwdEntry, 200, -1);
    gtk_box_pack_start(GTK_BOX(left), pwdEntry, FALSE, FALSE, 0);

    GtkWidget *slotLabel = gtk_label_new("CHOOSE YOUR SLOT");
    gtk_widget_set_name(slotLabel, "js-label");
    gtk_widget_set_halign(slotLabel, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(left), slotLabel, FALSE, FALSE, 0);

    slotCombo = gtk_combo_box_text_new();
    gtk_widget_set_name(slotCombo, "js-combo");
    gtk_widget_set_size_request(slotCombo, 200, -1);
    gtk_box_pack_start(GTK_BOX(left), slotCombo, FALSE, FALSE, 0);

    GtkWidget *spacing = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_vexpand(spacing, TRUE);
    gtk_box_pack_start(GTK_BOX(left), spacing, TRUE, TRUE, 0);

    // Confirm Join Button
    confirmButton = gtk_button_new_with_label("Confirm Join Request");
    gtk_widget_set_name(confirmButton, "js-invite");
    gtk_widget_set_size_request(confirmButton, 200, 52);
    gtk_box_pack_start(GTK_BOX(left), confirmButton, FALSE, FALSE, 0);
    g_signal_connect(confirmButton, "clicked", G_CALLBACK(onConfirmedJoinClicked), this);

    // right hand side for column
    GtkWidget *right = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_size_request(right, 220, -1);
    gtk_box_pack_start(GTK_BOX(dialogBox), right, FALSE, FALSE, 0);

    playerCountLabel = gtk_label_new("REGISTERED PLAYERS");
    gtk_widget_set_name(playerCountLabel, "js-section-label");
    gtk_widget_set_halign(playerCountLabel, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(right), playerCountLabel, FALSE, FALSE, 0);

    GtkWidget *makeHeader = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(right), makeHeader, FALSE, FALSE, 0);

    auto header = [](const char *text, int w) -> GtkWidget* {
        GtkWidget *sectionLabel = gtk_label_new(text);
        gtk_widget_set_name(sectionLabel, "js-section-label");
        gtk_widget_set_size_request(sectionLabel, w, -1);
        gtk_widget_set_halign(sectionLabel, GTK_ALIGN_START);
        return sectionLabel;
    };

    gtk_box_pack_start(GTK_BOX(makeHeader), header("Player", 100), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(makeHeader), header("Slot", 60), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(makeHeader), header("Join", 60), FALSE, FALSE, 0);

    GtkWidget *scroll = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scroll, 220, 150);
    gtk_box_pack_start(GTK_BOX(right), scroll, TRUE, TRUE, 0);

    listBox = gtk_list_box_new();
    gtk_widget_set_name(listBox, "js-player-list");
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(listBox), GTK_SELECTION_NONE);
    gtk_container_add(GTK_CONTAINER(scroll), listBox);

    // bottom space for lobby
    GtkWidget *bottomBar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_bottom(bottomBar, 15);
    gtk_box_pack_start(GTK_BOX(background), bottomBar, FALSE, FALSE, 8);

    lobbyButton = gtk_button_new_with_label("Lobby");
    gtk_widget_set_name(lobbyButton, "js-lobby");
    gtk_widget_set_size_request(lobbyButton, 200, 50);
    gtk_widget_set_halign(lobbyButton, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(bottomBar), lobbyButton, FALSE, FALSE, 0);
    g_signal_connect(lobbyButton, "clicked", G_CALLBACK(lobbyClicked), this);
}

void joinScreen::applyStyles() {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, joinCSS, -1, nullptr);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);
}

void joinScreen::playerList(const vector<PLAYER>& players, int maxPlayers) {
    GList *rows = gtk_container_get_children(GTK_CONTAINER(listBox));
    for (GList* i = rows; i; i = i -> next) {
        gtk_widget_destroy(GTK_WIDGET(i -> data));
    }
    g_list_free(rows);

    for (auto& p : players) {
        GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_name(row, "js-player-row");

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
}

void joinScreen::openSlots(const vector<int>& slots) {
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(slotCombo));
    for (int s : slots) {
        string label = "Slot " + to_string(s);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(slotCombo), label.c_str());
    }
    if (!slots.empty()) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(slotCombo), 0);
    }
}

void joinScreen::onConfirmedJoinClicked(GtkButton*, gpointer inputData) {
    auto *self = static_cast<joinScreen*>(inputData);
    if (!self->onConfirmedJoin) return;

    string user = gtk_entry_get_text(GTK_ENTRY(self->userEntry));
    string pass = gtk_entry_get_text(GTK_ENTRY(self->pwdEntry));
    
    char* activeText = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(self->slotCombo));
    int selectedSlot = -1;
    
    if (activeText != nullptr) {
        string s(activeText);
        if (s.find("Slot ") != string::npos) {
            selectedSlot = stoi(s.substr(5));
        }
        g_free(activeText);
    }

    extern LOGININFO playerLoginInfo;

    // Additional logic needed for merging
    strncpy(playerLoginInfo.playerName, user.c_str(), sizeof(playerLoginInfo.playerName) -1);
    strncpy(playerLoginInfo.password, pass.c_str(), sizeof(playerLoginInfo.password)-1);
    playerLoginInfo.playerNum = selectedSlot;
    playerLoginInfo.playerType = Human;

    if (selectedSlot >= 0){
        if (selectedSlot >= 0 && (size_t)selectedSlot < playerLoginInfo.playersFound.size()) {
            playerLoginInfo.playersFound[selectedSlot] = 1;
        }
        playerLoginInfo.playersFound[selectedSlot] = 1;
    }

    //if (!self -> onConfirmedJoin) return;
    
    self->onConfirmedJoin(user, pass, selectedSlot);
}

void joinScreen::lobbyClicked(GtkButton*, gpointer inputData) {
    auto *self = static_cast<joinScreen*>(inputData);
    if (self->onLobby) {
        self->onLobby();
    }
}

#ifdef TEST_JOIN
int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Tester Join Screen Layout");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    joinScreen joinScreen;
    
    vector<RegisteredPlayer> mockPlayers = {{"Alice", 1}, {"Bob", 2}};
    joinScreen.playerList(mockPlayers, 4);

    vector<int> mockAvailableSlots = {3, 4};
    joinScreen.openSlots(mockAvailableSlots);

    gtk_container_add(GTK_CONTAINER(window), joinScreen.getWidget());

    gtk_widget_show_all(window);
    gtk_main();
    
    return 0;
}
#endif