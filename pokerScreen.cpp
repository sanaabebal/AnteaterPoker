// pokerScreen.cpp
#include <cmath>
#include <sstream>
#include <iomanip>
#include <vector>
#include <iostream>
// CODEX FIX: removed <filesystem>; using GLib's g_file_test() below for server compatibility.
#include "pokerScreen.hpp"

using namespace std;

// ── Application CSS Definitions ───────────────────────────────────────────
static const char* GAME_CSS = R"CSS(
.gs-root {
    background-color: #1a5c5a;
}
.gs-pot-label {
    color: #f5ecc8;
    font-family: "Georgia", serif;
    font-size: 15px;
    font-weight: bold;
}
.gs-action-bar {
    background-color: #111f1e;
    padding: 6px;
}
.gs-fold-btn {
    background-color: #2e5c2e;
    color: #f5ecc8;
    font-family: "Georgia", serif;
    font-size: 14px;
    font-weight: bold;
    border-radius: 4px;
    border: 2px solid #1a3a1a;
    padding: 10px 32px;
    min-width: 120px;
}

.gs-fold-btn:hover { 
    background-color: #3a7a3a; 
}

.gs-check-btn {
    background-color: #2e5c2e;
    color: #f5ecc8;
    font-family: "Georgia", serif;
    font-size: 14px;
    font-weight: bold;
    border-radius: 4px;
    border: 2px solid #1a3a1a;
    padding: 10px 32px;
    min-width: 120px;
}
.gs-check-btn:hover { background-color: #3a7a3a; }
.gs-bet-btn {
    background-color: #2e5c2e;
    color: #f5ecc8;
    font-family: "Georgia", serif;
    font-size: 14px;
    font-weight: bold;
    border-radius: 4px;
    border: 2px solid #1a3a1a;
    padding: 10px 16px;
    min-width: 60px;
}
.gs-bet-btn:hover { background-color: #3a7a3a; }
.gs-spin {
    background-color: #1e3a38;
    color: #f5ecc8;
    font-family: "Georgia", serif;
    font-size: 13px;
    border: 1px solid #3a6a68;
    min-width: 70px;
}
.gs-allin-btn {
    background-color: #2e5c2e;
    color: #f5ecc8;
    font-family: "Georgia", serif;
    font-size: 14px;
    font-weight: bold;
    border-radius: 4px;
    border: 2px solid #1a3a1a;
    padding: 10px 32px;
    min-width: 120px;
}
.gs-allin-btn:hover {
    background-color: #3a7a3a;
}
/* CODEX FIX: style the dealer-only next-hand button in the same action bar family. */
.gs-start-hand-btn {
    background-color: #8a6f22;
    color: #f5ecc8;
    font-family: "Georgia", serif;
    font-size: 14px;
    font-weight: bold;
    border-radius: 4px;
    border: 2px solid #5f4a12;
    padding: 10px 24px;
    min-width: 130px;
}
/* CODEX FIX: make the next-hand button visibly interactive when the dealer can press it. */
.gs-start-hand-btn:hover {
    background-color: #a8882a;
}

)CSS";

static string resolveFile(const string& relativePath) {

    vector<string> searchPaths = {
        relativePath,
        "../" + relativePath,
        "../../" + relativePath,
        "../../../" + relativePath
    };

    for (const auto& path : searchPaths) {
        // CODEX FIX: replaced filesystem::exists(path) so this builds without C++17 filesystem support.
        if (g_file_test(path.c_str(), G_FILE_TEST_EXISTS)) {
            return path;
        }
    }

    cerr << "File not found: " << relativePath << endl;
    return relativePath;
}

static string getCardFilePath(const Card& card) {

    if (card.val < 0 || card.suit < 0 || card.val > 12 || card.suit > 3) {
        return resolveFile("assets/Back.png");
    }


    string suitName;
    if (card.suit == 0)      suitName = "Heart";
    else if (card.suit == 1) suitName = "Clover";   
    else if (card.suit == 2) suitName = "Diamond";
    else if (card.suit == 3) suitName = "Spades";
    else {
        return resolveFile("assets/Anteater.png");
    }

    const char* ranks[] = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King", "Ace" };
    string rankNum = ranks[card.val];

    string path = "assets/" + suitName + "/" + suitName + " " + rankNum + ".png";

    return resolveFile(path);
}


static void setRGB(cairo_t* cr, double r, double g, double b) {
    cairo_set_source_rgb(cr, r, g, b);
}
static void setRGBA(cairo_t* cr, double r, double g, double b, double a) {
    cairo_set_source_rgba(cr, r, g, b, a);
}

pokerScreen::pokerScreen() {
    buildUI();
    applyStyles();
}

pokerScreen::~pokerScreen() {}

GtkWidget* pokerScreen::getWidget() { 
    return container; 
}

void pokerScreen::buildUI() {
    container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(container, "gs-root");
    gtk_widget_set_hexpand(container, TRUE);
    gtk_widget_set_vexpand(container, TRUE);

    tableDrawingArea = gtk_drawing_area_new();
    gtk_widget_set_hexpand(tableDrawingArea, TRUE);
    gtk_widget_set_vexpand(tableDrawingArea, TRUE);
    gtk_widget_set_size_request(tableDrawingArea, 520, 340);
    gtk_box_pack_start(GTK_BOX(container), tableDrawingArea, TRUE, TRUE, 0);
    g_signal_connect(tableDrawingArea, "draw", G_CALLBACK(onDraw), this);

    GtkWidget* actionBar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_name(actionBar, "gs-action-bar");
    gtk_widget_set_margin_top(actionBar, 4);
    gtk_widget_set_margin_bottom(actionBar, 4);
    gtk_widget_set_margin_start(actionBar, 12);
    gtk_widget_set_margin_end(actionBar, 12);
    gtk_box_pack_start(GTK_BOX(container), actionBar, FALSE, FALSE, 0);

    foldButton = gtk_button_new_with_label("FOLD");
    gtk_widget_set_name(foldButton, "gs-fold-btn");
    //gtk_widget_set_halign(foldButton, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(actionBar), foldButton, TRUE, TRUE, 0);
    g_signal_connect(foldButton, "clicked", G_CALLBACK(onFoldClicked), this);

    checkButton = gtk_button_new_with_label("CHECK");
    gtk_widget_set_name(checkButton, "gs-check-btn");
    //gtk_widget_set_halign(checkButton, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(actionBar), checkButton, TRUE, TRUE, 0);
    g_signal_connect(checkButton, "clicked", G_CALLBACK(onCheckClicked), this);

    GtkWidget* betBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_box_pack_start(GTK_BOX(actionBar), betBox, TRUE, TRUE, 0);

    potLabel = gtk_label_new("BET");
    gtk_widget_set_name(potLabel, "gs-pot-label");
    gtk_box_pack_start(GTK_BOX(betBox), potLabel, FALSE, FALSE, 4);

    GtkWidget* minusBtn = gtk_button_new_with_label("−");
    gtk_widget_set_name(minusBtn, "gs-bet-btn");
    gtk_widget_set_size_request(minusBtn, 30, -1);
    //gtk_widget_set_halign(minusBtn, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(betBox), minusBtn, FALSE, FALSE, 0);

    betSpinButton = gtk_spin_button_new_with_range(10, 100000, 50);
    gtk_widget_set_name(betSpinButton, "gs-spin");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(betSpinButton), 50);
    gtk_widget_set_size_request(betSpinButton, 80, -1);
    //gtk_widget_set_halign(betSpinButton, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(betBox), betSpinButton, FALSE, FALSE, 0);

    GtkWidget* plusBtn = gtk_button_new_with_label("+");
    gtk_widget_set_name(plusBtn, "gs-bet-btn");
    gtk_widget_set_size_request(plusBtn, 30, -1);
    //gtk_widget_set_halign(plusBtn, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(betBox), plusBtn, FALSE, FALSE, 0);

    g_signal_connect_swapped(minusBtn, "clicked",
        G_CALLBACK(+[](GtkSpinButton* s){ gtk_spin_button_spin(s, GTK_SPIN_STEP_BACKWARD, 1); }),
        betSpinButton);
    g_signal_connect_swapped(plusBtn, "clicked",
        G_CALLBACK(+[](GtkSpinButton* s){ gtk_spin_button_spin(s, GTK_SPIN_STEP_FORWARD, 1); }),
        betSpinButton);

    betButton = gtk_button_new_with_label("BET");
    gtk_widget_set_name(betButton, "gs-bet-btn");
    gtk_widget_set_size_request(betButton, 60, -1);
    //gtk_widget_set_halign(betButton, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(betBox), betButton, FALSE, FALSE, 0);
    g_signal_connect(betButton, "clicked", G_CALLBACK(onBetClicked), this);

    allInButton = gtk_button_new_with_label("ALL IN");
    gtk_widget_set_name(allInButton, "gs-allin-btn");
    //gtk_widget_set_halign(allInButton, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(actionBar), allInButton, TRUE, TRUE, 0);
    g_signal_connect(allInButton, "clicked", G_CALLBACK(onAllInClicked), this);

    // CODEX FIX: add a disabled-by-default dealer control for starting the next hand between hands.
    startHandButton = gtk_button_new_with_label("START HAND");
    gtk_widget_set_name(startHandButton, "gs-start-hand-btn");
    gtk_box_pack_start(GTK_BOX(actionBar), startHandButton, TRUE, TRUE, 0);
    gtk_widget_set_sensitive(startHandButton, FALSE); // CODEX FIX: only ClientGUI enables this for the dealer between hands.
    g_signal_connect(startHandButton, "clicked", G_CALLBACK(onStartHandClicked), this);

}

void pokerScreen::applyStyles() {
    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, GAME_CSS, -1, nullptr);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

void pokerScreen::updateGameState(const vector<PLAYER>& players,
                                  const vector<Card>& communityCards,
                                  const vector<Card>& holeCards,
                                  int pot,
                                  int currentBet,
                                  int currentPlayerStack,
                                  int playerTurn,
                                  int dealerTurn) {
    cachedPlayers.clear();
    for (const auto& enginePlayer : players) {
        playerInfo uiSeat;
        uiSeat.name = enginePlayer.name;
        uiSeat.stack = enginePlayer.score;
        uiSeat.avatarIndex = enginePlayer.playerNum;
        
        // Determine player status text based on engine flags
        if (enginePlayer.isInHand == 0) {
            uiSeat.status = "Folded";
        } else if (enginePlayer.isEliminated == 1) {
            uiSeat.status = "Eliminated";
        } else {
            uiSeat.status = "Active";
        }
        
        // Match active turn markers
        // Assuming your controller handles turn indicators, or defaults to false
        uiSeat.yourTurn = (enginePlayer.playerNum == playerTurn); 
        uiSeat.isDealer = (enginePlayer.playerNum == dealerTurn); 

        cachedPlayers.push_back(uiSeat);
    }

    cachedCommunity = communityCards;
    cachedHole = holeCards;
    cachedPot = pot;
    cachedBet = currentBet > 0 ? currentBet : 50;
    localStack = currentPlayerStack;

    int maxBet = (localStack > 0) ? localStack : 1; // CODEX FIX: keep the spin range valid for all-in/zero-stack players.
    int minBet = (maxBet < 10) ? 1 : 10; // CODEX FIX: avoid setting a minimum greater than the player's stack.
    if(cachedBet < minBet){
        cachedBet = minBet; // CODEX FIX: clamp the displayed bet into the safe GTK spin range.
    }
    if(cachedBet > maxBet){
        cachedBet = maxBet; // CODEX FIX: clamp the displayed bet into the safe GTK spin range.
    }
    gtk_spin_button_set_range(GTK_SPIN_BUTTON(betSpinButton), minBet, maxBet);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(betSpinButton), cachedBet);

    gtk_widget_queue_draw(tableDrawingArea);
}

void pokerScreen::setActions(bool enable) {
    gtk_widget_set_sensitive(foldButton, enable);
    gtk_widget_set_sensitive(checkButton, enable);
    gtk_widget_set_sensitive(betButton, enable);
    gtk_widget_set_sensitive(betSpinButton, enable);
    // CODEX FIX: disable All In with the rest of the action buttons during lobby-synced table setup.
    gtk_widget_set_sensitive(allInButton, enable);
}

// CODEX FIX: keep next-hand control independent from fold/check/bet turn controls.
void pokerScreen::setStartHandAction(bool enable) {
    gtk_widget_set_sensitive(startHandButton, enable);
}

gboolean pokerScreen::onDraw(GtkWidget* widget, cairo_t* cr, gpointer data) {
    auto* self = static_cast<pokerScreen*>(data);
    int w = gtk_widget_get_allocated_width(widget);
    int h = gtk_widget_get_allocated_height(widget);
    self->drawTable(cr, w, h);
    return FALSE;
}

void pokerScreen::drawTable(cairo_t* cr, int w, int h) {
    setRGB(cr, 0.10, 0.36, 0.35);
    cairo_paint(cr);

    double cx = w / 2.0, cy = h / 2.0;
    double rx = w * 0.44, ry = h * 0.38;

    setRGBA(cr, 0, 0, 0, 0.4);
    cairo_save(cr);
    cairo_translate(cr, cx + 4, cy + 6);
    cairo_scale(cr, rx, ry);
    cairo_arc(cr, 0, 0, 1, 0, 2 * M_PI);
    cairo_restore(cr);
    cairo_fill(cr);

    setRGB(cr, 0.38, 0.26, 0.08);
    cairo_save(cr);
    cairo_translate(cr, cx, cy); 
    cairo_scale(cr, rx + 18, ry + 14);
    cairo_arc(cr, 0, 0, 1, 0, 2 * M_PI);
    cairo_restore(cr);
    cairo_fill(cr);

    setRGB(cr, 0.13, 0.40, 0.25);
    cairo_save(cr);
    cairo_translate(cr, cx, cy); 
    cairo_scale(cr, rx, ry);
    cairo_arc(cr, 0, 0, 1, 0, 2 * M_PI);
    cairo_restore(cr);
    cairo_fill(cr);

    setRGBA(cr, 0.16, 0.48, 0.30, 0.5);
    cairo_save(cr);
    cairo_translate(cr, cx, cy - ry * 0.08);
    cairo_scale(cr, rx * 0.88, ry * 0.78);
    cairo_arc(cr, 0, 0, 1, 0, 2 * M_PI);
    cairo_restore(cr);
    cairo_fill(cr);

    ostringstream potStr;
    potStr << "POT: $" << cachedPot;
    cairo_select_font_face(cr, "Georgia", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 14);
    setRGB(cr, 0.96, 0.93, 0.78);
    cairo_text_extents_t te;
    cairo_text_extents(cr, potStr.str().c_str(), &te);
    cairo_move_to(cr, cx - te.width / 2.0, cy - ry * 0.28);
    cairo_show_text(cr, potStr.str().c_str());


    double cardW = w * 0.08, cardH = cardW * 1.45;
    double cardSpacing = cardW + 6;
    int numCommunity = (int)cachedCommunity.size();
    double totalCW = 5 * cardSpacing - 6;
    double startX  = cx - totalCW / 2.0;
    double cardY   = cy - cardH / 2.0 + ry * 0.05;

    for (int i = 0; i < numCommunity; ++i) {
        drawCard(cr, startX + i * cardSpacing, cardY, cardW, cardH, cachedCommunity[i]);
    }
    Card back;
    for (int i = numCommunity; i < 5; ++i) {
        drawCard(cr, startX + i * cardSpacing, cardY, cardW, cardH, back);
    }

    double hcY = cy + ry * 0.52;
    double hcX = cx - cardSpacing / 2.0 - cardW / 2.0;
    for (size_t i = 0; i < cachedHole.size(); ++i) {
        double angleVal = (i == 0) ? -0.08 : 0.08;
        cairo_save(cr);
        cairo_translate(cr, hcX + i * cardSpacing, hcY + cardH / 2.0);
        cairo_rotate(cr, angleVal);
        cairo_translate(cr, -cardW / 2.0, -cardH / 2.0);
        drawCard(cr, 0, 0, cardW * 1.1, cardH * 1.1, cachedHole[i]);
        cairo_restore(cr);
    }

    int numSeats = (int)cachedPlayers.size();
    for (int i = 0; i < numSeats; ++i) {
        if (cachedPlayers[i].isDealer) {
            double angleOffset = M_PI / 2.0;
            double angle       = angleOffset + (2.0 * M_PI * i / numSeats);
            double buttonX     = cx + (rx * 0.72) * cos(angle) + 24;
            double buttonY     = cy + (ry * 0.72) * sin(angle) + 24;

            setRGB(cr, 0.95, 0.95, 0.95);
            cairo_arc(cr, buttonX, buttonY, 10, 0, 2 * M_PI);
            cairo_fill(cr);
            setRGB(cr, 0.1, 0.1, 0.1);
            cairo_arc(cr, buttonX, buttonY, 10, 0, 2 * M_PI);
            cairo_set_line_width(cr, 1.2);
            cairo_stroke(cr);
            cairo_set_font_size(cr, 9);
            cairo_text_extents_t dte;
            cairo_text_extents(cr, "D", &dte);
            cairo_move_to(cr, buttonX - dte.width / 2.0, buttonY + dte.height / 2.0);
            cairo_show_text(cr, "D");
            break;
        }
    }

    for (int i = 0; i < numSeats; ++i) {
        drawPlayer(cr, w, h, cachedPlayers[i], i, numSeats);
    }
}

void pokerScreen::drawCard(cairo_t* cr, double x, double y,
                           double cw, double ch, const Card& card) {
    string filePath = getCardFilePath(card);

    GError* error = nullptr;
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(filePath.c_str(), &error);

    if (error) {
        g_clear_error(&error);
        
        setRGB(cr, 0.18, 0.18, 0.18); 
        cairo_rectangle(cr, x, y, cw, ch);
        cairo_fill(cr);
        
        setRGB(cr, 0.4, 0.4, 0.4);
        cairo_rectangle(cr, x, y, cw, ch);
        cairo_set_line_width(cr, 1.5);
        cairo_stroke(cr);
        
        setRGB(cr, 1.0, 1.0, 1.0);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 11);
        
        cairo_text_extents_t te;
        string displayText;
        if (card.val < 0 || card.suit < 0) {
            displayText = "BACK";
        } else {
            const char* suitsShort[] = { "H", "C", "D", "S" };
            const char* valuesShort[] = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A" };
            displayText = string(valuesShort[card.val]) + suitsShort[card.suit];
        }
        cairo_text_extents(cr, displayText.c_str(), &te);
        
        cairo_move_to(cr, x + (cw - te.width) / 2.0, y + (ch + te.height) / 2.0);
        cairo_show_text(cr, displayText.c_str());
        return;
    }

    int imgW = gdk_pixbuf_get_width(pixbuf);
    int imgH = gdk_pixbuf_get_height(pixbuf);
    
    int minX = imgW, maxX = 0, minY = imgH, maxY = 0;
    bool hasAlpha = gdk_pixbuf_get_has_alpha(pixbuf);
    
    if (hasAlpha) {
        int nChannels = gdk_pixbuf_get_n_channels(pixbuf);
        int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
        guchar* pixels = gdk_pixbuf_get_pixels(pixbuf);

        for (int row = 0; row < imgH; row++) {
            guchar* rowPtr = pixels + row * rowstride;
            for (int col = 0; col < imgW; col++) {
                guchar alpha = rowPtr[col * nChannels + (nChannels - 1)];
                if (alpha > 10) { 
                    if (col < minX) minX = col;
                    if (col > maxX) maxX = col;
                    if (row < minY) minY = row;
                    if (row > maxY) maxY = row;
                }
            }
        }
    }

    GdkPixbuf* croppedPixbuf = nullptr;

    if (maxX >= minX && maxY >= minY) {
        int cropW = maxX - minX + 1;
        int cropH = maxY - minY + 1;
        croppedPixbuf = gdk_pixbuf_new_subpixbuf(pixbuf, minX, minY, cropW, cropH);
    } else {
        // CODEX FIX: GTK returns gpointer here; cast it back to GdkPixbuf* for C++.
        croppedPixbuf = GDK_PIXBUF(g_object_ref(pixbuf)); // rely on the full image
    }

    GdkPixbuf* scaledPixbuf = gdk_pixbuf_scale_simple(
        croppedPixbuf, (int)cw, (int)ch, GDK_INTERP_BILINEAR
    );
    
    cairo_save(cr);
    gdk_cairo_set_source_pixbuf(cr, scaledPixbuf, x, y);
    cairo_paint(cr);
    cairo_restore(cr);

    g_object_unref(scaledPixbuf);
    g_object_unref(croppedPixbuf);
    g_object_unref(pixbuf);
}

void pokerScreen::drawPlayer(cairo_t* cr, int w, int h,
                             const playerInfo& p, int seatIndex, int numSeats) {
    double cx = w / 2.0, cy = h / 2.0;
    double rx = w * 0.44, ry = h * 0.38;

    double angleOffset = M_PI / 2.0;
    double angle = angleOffset + (2.0 * M_PI * seatIndex / numSeats);
    
    double px = cx + rx * 1.12 * cos(angle);
    double py = cy + ry * 1.16 * sin(angle);

    double boxW = 150, boxH = 65;
    double bx = px - boxW / 2.0, by = py - boxH / 2.0;

    bool isTurn = p.yourTurn;

    if (isTurn) {
        setRGBA(cr, 0.85, 0.73, 0.25, 0.95); 
    }
    else {
        setRGBA(cr, 0.12, 0.25, 0.25, 0.90);
    }

    cairo_rectangle(cr, bx, by, boxW, boxH);
    cairo_fill(cr);

    setRGBA(cr, 0.60, 0.60, 0.50, 0.85);
    cairo_set_line_width(cr, isTurn ? 3.0 : 1.2);
    cairo_rectangle(cr, bx, by, boxW, boxH);
    cairo_stroke(cr);

    double avR = 20; 
    double avX = bx + avR + 8, avY = by + boxH / 2.0;

    static const double avColours[5][3] = {
        {0.55, 0.35, 0.20}, {0.40, 0.40, 0.42}, {0.70, 0.55, 0.30},
        {0.30, 0.55, 0.70}, {0.55, 0.25, 0.55}
    };
    int ci = p.avatarIndex % 5;
    setRGB(cr, avColours[ci][0], avColours[ci][1], avColours[ci][2]);
    cairo_arc(cr, avX, avY, avR, 0, 2 * M_PI);
    cairo_fill(cr);

    double textStartX = bx + (avR * 2) + 14;

    cairo_select_font_face(cr, "Georgia", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 12);
    setRGB(cr, 0.98, 0.96, 0.85);
    cairo_move_to(cr, textStartX, by + 18);
    cairo_show_text(cr, p.name.c_str());

    cairo_set_font_size(cr, 10.5);
    setRGBA(cr, 0.85, 0.85, 0.78, 0.95);
    cairo_move_to(cr, textStartX, by + 34);
    string statusStr = "Status: " + p.status;
    cairo_show_text(cr, statusStr.c_str());

    cairo_set_font_size(cr, 10.5);
    setRGB(cr, 0.80, 0.98, 0.80);
    cairo_move_to(cr, textStartX, by + 50);
    ostringstream stackStr;
    stackStr << "Stack: $" << p.stack;
    cairo_show_text(cr, stackStr.str().c_str());

    if (p.status == "Folded") {
        setRGBA(cr, 0.05, 0.05, 0.05, 0.75); 
        cairo_rectangle(cr, bx, by, boxW, boxH);
        cairo_fill(cr);
        
        setRGBA(cr, 0.9, 0.25, 0.25, 1.0);
        cairo_select_font_face(cr, "Georgia", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 14);
        cairo_text_extents_t fte;
        cairo_text_extents(cr, "FOLDED", &fte);
        cairo_move_to(cr, bx + (boxW - fte.width) / 2.0, by + (boxH + fte.height) / 2.0);
        cairo_show_text(cr, "FOLDED");
    }
}

void pokerScreen::onFoldClicked(GtkButton*, gpointer data) {
    auto* self = static_cast<pokerScreen*>(data);
    if (self->onFold) {
        self->onFold();
    }
}

void pokerScreen::onCheckClicked(GtkButton*, gpointer data) {
    auto* self = static_cast<pokerScreen*>(data);
    if (self->onCheck) {
        self->onCheck();
    }
}

void pokerScreen::onBetClicked(GtkButton*, gpointer data) {
    auto* self = static_cast<pokerScreen*>(data);
    if (!self->onBet) {
        return;
    }
    int amount = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(self->betSpinButton));
    self->onBet(amount);
}

void pokerScreen::onAllInClicked(GtkButton*, gpointer data) {
    auto* self = static_cast<pokerScreen*>(data);
    if (self->onAllIn) {
        self->onAllIn();
    }
}

void pokerScreen::onStartHandClicked(GtkButton*, gpointer data) {
    auto* self = static_cast<pokerScreen*>(data);
    if (self->onStartHand) {
        // CODEX FIX: delegate dealer start-hand requests back to ClientGUI/server validation.
        self->onStartHand();
    }
}


#ifdef TEST_POKER

struct TestPlayerInstance {
    int score;
    bool isStillActive;
};

struct TestGameEngineState {
    int potTotal;
    int minimumToCall;
    int dealerIndex;
    int actorTurnIndex;
    vector<TestPlayerInstance> seats;
};

int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Texas Hold'em - Independent Sandbox Workspace");
    gtk_window_set_default_size(GTK_WINDOW(window), 820, 620);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), nullptr);

    pokerScreen gameUI;
    gtk_container_add(GTK_CONTAINER(window), gameUI.getWidget());

    TestGameEngineState mockData;
    mockData.potTotal = 750;
    mockData.minimumToCall = 150;
    mockData.dealerIndex = 0; 
    mockData.actorTurnIndex = 1; 

    mockData.seats = {
        {3400, true},  
        {1850, true},  
        {900,  false}  
    };

    vector<playerInfo> finalPlayersList;
    for (size_t i = 0; i < mockData.seats.size(); ++i) {
        playerInfo uiSeat;
        uiSeat.name = "Player " + to_string(i + 1);
        uiSeat.stack = mockData.seats[i].score;
        uiSeat.avatarIndex = (int)i;
        uiSeat.isDealer = (mockData.dealerIndex == (int)i);
        uiSeat.yourTurn = (mockData.actorTurnIndex == (int)i);
        uiSeat.status = mockData.seats[i].isStillActive ? (uiSeat.yourTurn ? "Active..." : "Waiting") : "Folded";
        finalPlayersList.push_back(uiSeat);
    }

    vector<Card> communityBoard;
    // enum val { Two... Ten=8, Jack=9, Queen=10, King=11, Ace=12 }
    // enum suit { Hearts=0, Clubs=1, Diamonds=2, Spades=3 }
    Card board1(8, 0);  // 10 of Hearts
    Card board2(9, 0);  // Jack of Hearts
    Card board3(10, 0); // Queen of Hearts
    communityBoard.push_back(board1);
    communityBoard.push_back(board2);
    communityBoard.push_back(board3);

    vector<Card> privateHoleCards;
    Card hole1(12, 0);  // Ace of Hearts
    Card hole2(11, 0);  // King of Hearts
    privateHoleCards.push_back(hole1);
    privateHoleCards.push_back(hole2);

    gameUI.onFold  = []() { cout << "[Sandbox Action Log]: FOLD was pressed!" << endl; };
    gameUI.onCheck = []() { cout << "[Sandbox Action Log]: CHECK/CALL was pressed!" << endl; };
    gameUI.onBet   = [](int val) { cout << "[Sandbox Action Log]: BET registration parsed for: $" << val << endl; };
    gameUI.onAllIn = []() { cout << "[Sandbox Action Log]: ALL IN was pressed!" << endl; };

    gameUI.updateGameState(finalPlayersList, communityBoard, privateHoleCards, mockData.potTotal, mockData.minimumToCall, 2500);
    gameUI.setActions(true);

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}

#endif
