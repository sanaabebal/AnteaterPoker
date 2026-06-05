#include "gameOverScreen.hpp"
#include <sstream>
#include <cmath>

static const char* GAMEOVER_CSS = R"CSS(
.go-root {
    background-color: #1a1a1a;
}
.go-header {
    color: #d4b96a;
    font-family: "Georgia", serif;
    font-size: 20px;
    font-weight: bold;
    letter-spacing: 3px;
}
.go-suit-label {
    color: #c0392b;
    font-family: "Georgia", serif;
    font-size: 16px;
}
.go-winner-badge {
    background-color: #b8860b;
    color: #f5ecc8;
    font-family: "Georgia", serif;
    font-size: 11px;
    font-weight: bold;
    border-radius: 3px;
    padding: 1px 6px;
}
.go-player-row {
    background-color: #2a2a2a;
    border: 1px solid #444;
    border-radius: 4px;
}
.go-rank-label {
    color: #f5ecc8;
    font-family: "Georgia", serif;
    font-size: 14px;
    font-weight: bold;
}
.go-player-name {
    color: #f5ecc8;
    font-family: "Georgia", serif;
    font-size: 13px;
}
.go-chips-label {
    color: #a0c0a0;
    font-family: "Georgia", serif;
    font-size: 12px;
}
.go-delta-label {
    color: #60c060;
    font-family: "Georgia", serif;
    font-size: 12px;
    font-weight: bold;
}
.go-summary-title {
    color: #d4b96a;
    font-family: "Georgia", serif;
    font-size: 13px;
    font-weight: bold;
    letter-spacing: 1px;
}
.go-summary-label {
    color: #c0c0c0;
    font-family: "Georgia", serif;
    font-size: 12px;
}
.go-summary-value {
    color: #f5ecc8;
    font-family: "Georgia", serif;
    font-size: 12px;
    font-weight: bold;
}
.go-bigpot-value {
    color: #e05050;
    font-family: "Georgia", serif;
    font-size: 12px;
    font-weight: bold;
}
.go-btn {
    background-color: #2e5c2e;
    color: #f5ecc8;
    font-family: "Georgia", serif;
    font-size: 13px;
    font-weight: bold;
    border-radius: 4px;
    border: 2px solid #1a3a1a;
    padding: 8px 24px;
    min-width: 140px;
}
.go-btn:hover { background-color: #3a7a3a; }
)CSS";

// ── helpers (shared with GameScreen) ─────────────────────────────────────
static void goSetRGB(cairo_t* cr, double r, double g, double b){
    cairo_set_source_rgb(cr,r,g,b);
}
static void goSetRGBA(cairo_t* cr, double r, double g, double b, double a){
    cairo_set_source_rgba(cr,r,g,b,a);
}

static void drawOneCard(cairo_t* cr, double x, double y, double cw, double ch,
                        const std::string& rank, const std::string& suit) {
    double radius = 5.0;
    // Shadow
    goSetRGBA(cr, 0,0,0,0.35);
    cairo_rectangle(cr, x+3, y+3, cw, ch);
    cairo_fill(cr);

    // Face
    goSetRGB(cr, 0.97, 0.97, 0.94);
    cairo_new_sub_path(cr);
    cairo_arc(cr, x+cw-radius, y+radius,    radius, -M_PI/2, 0);
    cairo_arc(cr, x+cw-radius, y+ch-radius, radius,  0,       M_PI/2);
    cairo_arc(cr, x+radius,    y+ch-radius, radius,  M_PI/2,  M_PI);
    cairo_arc(cr, x+radius,    y+radius,    radius,  M_PI,    3*M_PI/2);
    cairo_close_path(cr);
    cairo_fill_preserve(cr);
    goSetRGBA(cr, 0,0,0,0.25);
    cairo_set_line_width(cr, 0.8);
    cairo_stroke(cr);

    bool isRed = (suit == "♥" || suit == "♦");
    if (isRed) goSetRGB(cr, 0.85, 0.10, 0.10);
    else       goSetRGB(cr, 0.05, 0.05, 0.05);

    cairo_select_font_face(cr, "Georgia", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, cw * 0.34);
    cairo_move_to(cr, x + 4, y + cw * 0.35);
    cairo_show_text(cr, rank.c_str());

    cairo_set_font_size(cr, cw * 0.48);
    cairo_text_extents_t te;
    cairo_text_extents(cr, suit.c_str(), &te);
    cairo_move_to(cr, x + cw/2.0 - te.width/2.0, y + ch/2.0 + te.height/2.0);
    cairo_show_text(cr, suit.c_str());

    // Bottom rank (rotated)
    cairo_save(cr);
    cairo_translate(cr, x + cw - 3, y + ch - 4);
    cairo_rotate(cr, M_PI);
    cairo_set_font_size(cr, cw * 0.34);
    cairo_move_to(cr, 0, 0);
    cairo_show_text(cr, rank.c_str());
    cairo_restore(cr);
}

// ── Static draw callbacks (lambdas can't be passed to G_CALLBACK portably) ──

static gboolean drawChipStack(GtkWidget* w, cairo_t* cr, gpointer) {
    int cw = gtk_widget_get_allocated_width(w);
    int ch = gtk_widget_get_allocated_height(w);
    double cx = cw / 2.0, cy = ch * 0.75;
    static const double colours[4][3] = {
        {0.18,0.18,0.80},
        {0.80,0.18,0.18},
        {0.18,0.70,0.18},
        {0.70,0.70,0.18}
    };
    for (int s = 0; s < 4; ++s) {
        double sx = cx + (s - 1.5) * 28;
        int chips = 5 + s * 2;
        for (int c = chips - 1; c >= 0; --c) {
            cairo_set_source_rgb(cr,
                colours[s][0] * (0.7 + 0.06*c),
                colours[s][1] * (0.7 + 0.06*c),
                colours[s][2] * (0.7 + 0.06*c));
            cairo_arc(cr, sx, cy - c * 4, 12, 0, 2*M_PI);
            cairo_fill_preserve(cr);
            goSetRGBA(cr,1,1,1,0.12);
            cairo_set_line_width(cr,0.8);
            cairo_stroke(cr);
        }
    }
    cairo_set_source_rgb(cr,0.18,0.48,0.18);
    cairo_rectangle(cr, cx - 16, cy - 56, 32, 20);
    cairo_fill(cr);
    cairo_set_source_rgb(cr,0.5,0.9,0.3);
    cairo_select_font_face(cr,"Georgia",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr,12);
    cairo_move_to(cr, cx - 5, cy - 41);
    cairo_show_text(cr,"$");
    return FALSE;
}

static gboolean drawAvatar(GtkWidget* w, cairo_t* cr, gpointer) {
    int sz = gtk_widget_get_allocated_width(w);
    goSetRGB(cr, 0.25, 0.25, 0.28);
    cairo_arc(cr, sz/2.0, sz/2.0, sz/2.0 - 1, 0, 2*M_PI);
    cairo_fill(cr);
    goSetRGBA(cr,1,1,1,0.15);
    cairo_arc(cr, sz/2.0, sz/2.0, sz/2.0 - 1, 0, 2*M_PI);
    cairo_set_line_width(cr,1.5);
    cairo_stroke(cr);
    goSetRGB(cr,0.1,0.1,0.1);
    cairo_arc(cr, sz*0.36, sz*0.46, sz*0.14, 0, 2*M_PI);
    cairo_fill(cr);
    cairo_arc(cr, sz*0.64, sz*0.46, sz*0.14, 0, 2*M_PI);
    cairo_fill(cr);
    return FALSE;
}

// ── constructor ───────────────────────────────────────────────────────────
gameOverScreen::gameOverScreen() {
    buildUI();
    applyStyles();
}

gameOverScreen::~gameOverScreen() {}

GtkWidget* gameOverScreen::getWidget() { return container; }

// ── buildUI ───────────────────────────────────────────────────────────────
void gameOverScreen::buildUI() {
    container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(container, "go-root");
    gtk_widget_set_hexpand(container, TRUE);
    gtk_widget_set_vexpand(container, TRUE);

    // ── Header ────────────────────────────────────────────────────────────
    GtkWidget* headerBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_halign(headerBox, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(headerBox, 16);
    gtk_widget_set_margin_bottom(headerBox, 12);
    gtk_box_pack_start(GTK_BOX(container), headerBox, FALSE, FALSE, 0);

    // Suit decorations + title
    auto makeSuit = [](const char* s) -> GtkWidget* {
        GtkWidget* l = gtk_label_new(s);
        gtk_widget_set_name(l, "go-suit-label");
        return l;
    };
    gtk_box_pack_start(GTK_BOX(headerBox), makeSuit("♥ ♦"), FALSE, FALSE, 4);

    GtkWidget* titleLabel = gtk_label_new("GAME OVER – FINAL SCORES");
    gtk_widget_set_name(titleLabel, "go-header");
    gtk_box_pack_start(GTK_BOX(headerBox), titleLabel, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(headerBox), makeSuit("♠ ♣"), FALSE, FALSE, 4);

    // ── Body: leaderboard + summary side-by-side ──────────────────────────
    GtkWidget* bodyBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_set_margin_start(bodyBox, 24);
    gtk_widget_set_margin_end(bodyBox, 24);
    gtk_widget_set_vexpand(bodyBox, TRUE);
    gtk_box_pack_start(GTK_BOX(container), bodyBox, TRUE, TRUE, 0);

    // LEFT: results list
    resultBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_size_request(resultBox, 300, -1);
    gtk_box_pack_start(GTK_BOX(bodyBox), resultBox, FALSE, FALSE, 0);

    // Winner hand drawing area (below results list, for the winner's cards)
    winnerHandArea = gtk_drawing_area_new();
    gtk_widget_set_size_request(winnerHandArea, 300, 80);
    gtk_box_pack_start(GTK_BOX(bodyBox), winnerHandArea, FALSE, FALSE, 0);
    g_signal_connect(winnerHandArea, "draw", G_CALLBACK(onDrawHand), this);

    // RIGHT: session summary
    GtkWidget* summaryFrame = gtk_frame_new(nullptr);
    gtk_widget_set_size_request(summaryFrame, 200, -1);
    gtk_widget_set_valign(summaryFrame, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(bodyBox), summaryFrame, FALSE, FALSE, 0);

    summaryBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_margin_top(summaryBox, 12);
    gtk_widget_set_margin_bottom(summaryBox, 12);
    gtk_widget_set_margin_start(summaryBox, 12);
    gtk_widget_set_margin_end(summaryBox, 12);
    gtk_container_add(GTK_CONTAINER(summaryFrame), summaryBox);

    GtkWidget* sumTitle = gtk_label_new("SESSION SUMMARY");
    gtk_widget_set_name(sumTitle, "go-summary-title");
    gtk_widget_set_halign(sumTitle, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(summaryBox), sumTitle, FALSE, FALSE, 0);

    // Chip stack illustration in summary area
    GtkWidget* chipArea = gtk_drawing_area_new();
    gtk_widget_set_size_request(chipArea, 180, 90);
    gtk_widget_set_halign(chipArea, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(summaryBox), chipArea, FALSE, FALSE, 0);
    g_signal_connect(chipArea, "draw", G_CALLBACK(drawChipStack), nullptr);

    // ── Bottom button bar ─────────────────────────────────────────────────
    GtkWidget* btnBar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_set_halign(btnBar, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(btnBar, 12);
    gtk_widget_set_margin_bottom(btnBar, 16);
    gtk_box_pack_start(GTK_BOX(container), btnBar, FALSE, FALSE, 0);

    playAgainButton = gtk_button_new_with_label("PLAY AGAIN");
    gtk_widget_set_name(playAgainButton, "go-btn");
    gtk_box_pack_start(GTK_BOX(btnBar), playAgainButton, FALSE, FALSE, 0);
    // Point to onPlayAgainClicked
    g_signal_connect(playAgainButton, "clicked", G_CALLBACK(onPlayAgainClicked), this);

    exitLobbyButton = gtk_button_new_with_label("EXIT TO LOBBY");
    gtk_widget_set_name(exitLobbyButton, "go-btn");
    gtk_box_pack_start(GTK_BOX(btnBar), exitLobbyButton, FALSE, FALSE, 0);
    // Point to onExitLobbyClicked
    g_signal_connect(exitLobbyButton, "clicked", G_CALLBACK(onExitLobbyClicked), this);
}

void gameOverScreen::applyStyles() {
    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, GAMEOVER_CSS, -1, nullptr);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

// ── setResults ────────────────────────────────────────────────────────────
void gameOverScreen::setResults(const std::vector<FinalPlayerResults>& results,
                                const gameSummary&                     summary) {
    populateResults(results);
    populateSummary(summary);

    // Cache winner's cards for the drawing area
    winnerCards.clear();
    for (auto& r : results) {
        if (r.isWinner) {
            winnerCards = r.handCards;
            break;
        }
    }
    gtk_widget_queue_draw(winnerHandArea);
}

void gameOverScreen::populateResults(const std::vector<FinalPlayerResults>& results) {
    // Clear old rows
    GList* children = gtk_container_get_children(GTK_CONTAINER(resultBox));
    for (GList* l = children; l; l = l->next)
        gtk_widget_destroy(GTK_WIDGET(l->data));
    g_list_free(children);

    for (auto& r : results) {
        // Row container
        GtkWidget* rowFrame = gtk_frame_new(nullptr);
        gtk_widget_set_name(rowFrame, "go-player-row");
        gtk_box_pack_start(GTK_BOX(resultBox), rowFrame, FALSE, FALSE, 0);

        GtkWidget* rowBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
        gtk_widget_set_margin_top(rowBox, 8);
        gtk_widget_set_margin_bottom(rowBox, 8);
        gtk_widget_set_margin_start(rowBox, 10);
        gtk_widget_set_margin_end(rowBox, 10);
        gtk_container_add(GTK_CONTAINER(rowFrame), rowBox);

        // Avatar placeholder (dark circle)
        GtkWidget* avatarArea = gtk_drawing_area_new();
        gtk_widget_set_size_request(avatarArea, 36, 36);
        g_signal_connect(avatarArea, "draw", G_CALLBACK(drawAvatar), nullptr);
        gtk_box_pack_start(GTK_BOX(rowBox), avatarArea, FALSE, FALSE, 0);

        // Rank + name + chips
        GtkWidget* textCol = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        gtk_box_pack_start(GTK_BOX(rowBox), textCol, TRUE, TRUE, 0);

        std::ostringstream nameStr;
        nameStr << r.rank << ".  " << r.username;
        GtkWidget* nameLabel = gtk_label_new(nameStr.str().c_str());
        gtk_widget_set_name(nameLabel, "go-rank-label");
        gtk_widget_set_halign(nameLabel, GTK_ALIGN_START);
        gtk_box_pack_start(GTK_BOX(textCol), nameLabel, FALSE, FALSE, 0);

        std::ostringstream chipsStr;
        chipsStr << "CHIPS: " << r.chips;
        if (r.chipDelta > 0) chipsStr << "  (+" << r.chipDelta << ")";
        GtkWidget* chipsLabel = gtk_label_new(chipsStr.str().c_str());
        gtk_widget_set_name(chipsLabel, r.chipDelta > 0 ? "go-delta-label" : "go-chips-label");
        gtk_widget_set_halign(chipsLabel, GTK_ALIGN_START);
        gtk_box_pack_start(GTK_BOX(textCol), chipsLabel, FALSE, FALSE, 0);

        // WINNER badge
        if (r.isWinner) {
            GtkWidget* badge = gtk_label_new("WINNER");
            gtk_widget_set_name(badge, "go-winner-badge");
            gtk_widget_set_valign(badge, GTK_ALIGN_CENTER);
            gtk_box_pack_end(GTK_BOX(rowBox), badge, FALSE, FALSE, 4);
        }
    }
    gtk_widget_show_all(resultBox);
}

void gameOverScreen::populateSummary(const gameSummary& s) {
    // Remove old stat rows (keep title + chipArea = first 2 children)
    GList* children = gtk_container_get_children(GTK_CONTAINER(summaryBox));
    int idx = 0;
    for (GList* l = children; l; l = l->next, ++idx) {
        if (idx >= 2) gtk_widget_destroy(GTK_WIDGET(l->data));
    }
    g_list_free(children);

    auto addRow = [this](const std::string& label, const std::string& value, bool bigPot = false) {
        GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_pack_start(GTK_BOX(summaryBox), row, FALSE, FALSE, 0);

        GtkWidget* lbl = gtk_label_new(label.c_str());
        gtk_widget_set_name(lbl, "go-summary-label");
        gtk_widget_set_halign(lbl, GTK_ALIGN_START);
        gtk_box_pack_start(GTK_BOX(row), lbl, TRUE, TRUE, 0);

        GtkWidget* val = gtk_label_new(value.c_str());
        gtk_widget_set_name(val, bigPot ? "go-bigpot-value" : "go-summary-value");
        gtk_widget_set_halign(val, GTK_ALIGN_END);
        gtk_box_pack_start(GTK_BOX(row), val, FALSE, FALSE, 0);
    };

    addRow("Total Hands", std::to_string(s.totalHands));
    addRow("Duration", s.duration);
    std::ostringstream potStr;
    potStr << s.biggestPot;
    addRow("Biggest Pot", potStr.str(), true);

    gtk_widget_show_all(summaryBox);
}

// ── Cairo: winner hand ────────────────────────────────────────────────────
gboolean gameOverScreen::onDrawHand(GtkWidget* widget, cairo_t* cr, gpointer data) {
    auto* self = static_cast<gameOverScreen*>(data);
    int w = gtk_widget_get_allocated_width(widget);
    int h = gtk_widget_get_allocated_height(widget);
    self->drawnHandCards(cr, w, h);
    return FALSE;
}

void gameOverScreen::drawnHandCards(cairo_t* cr, int w, int h) {
    if (winnerCards.empty()) return;

    double cardW = 44, cardH = 64;
    double spacing = cardW + 6;
    int n = (int)winnerCards.size();
    double totalW = n * spacing - 6;
    double startX = w / 2.0 - totalW / 2.0;
    double startY = (h - cardH) / 2.0;

    for (int i = 0; i < n; ++i) {
        drawOneCard(cr,
                    startX + i * spacing, startY,
                    cardW, cardH,
                    winnerCards[i].first,
                    winnerCards[i].second);
    }
}

// ── static signal handlers ────────────────────────────────────────────────
void gameOverScreen::onPlayAgainClicked(GtkButton*, gpointer data) {
    auto* self = static_cast<gameOverScreen*>(data);
    // Fires the engine's public std::function variable safely if bound
    if (self->onPlayAgain) self->onPlayAgain();
}

void gameOverScreen::onExitLobbyClicked(GtkButton*, gpointer data) {
    auto* self = static_cast<gameOverScreen*>(data);
    if (self->onExitToLobby) self->onExitToLobby();
    else gtk_main_quit(); // Fallback if no engine hook is configured
}

#ifdef TEST_GAMEOVER_SCREEN

#include <iostream>

static void handlePlayAgain() {
    std::cout << "[TEST] 'Play Again' hook triggered successfully!" << std::endl;
}

int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);

    gameOverScreen screenInstance;

    screenInstance.onPlayAgain = handlePlayAgain;

    // Build Mock Data matching structure names in gameOverScreen.hpp
    std::vector<FinalPlayerResults> mockResults;
    
    FinalPlayerResults p1;
    p1.rank = 1;
    p1.username = "AcesHigh_99";
    p1.chips = 24500;
    p1.chipDelta = 12500;
    p1.isWinner = true;
    p1.handCards = {{"A", "♠"}, {"A", "♦"}, {"10", "♠"}, {"J", "♠"}, {"Q", "♠"}};
    mockResults.push_back(p1);

    FinalPlayerResults p2;
    p2.rank = 2;
    p2.username = "BluffMaster";
    p2.chips = 8000;
    p2.chipDelta = -3000;
    p2.isWinner = false;
    mockResults.push_back(p2);

    gameSummary mockSummary;
    mockSummary.totalHands = 42;
    mockSummary.duration = "1h 24m 05s";
    mockSummary.biggestPot = 18500;

    // Load data structures into view
    screenInstance.setResults(mockResults, mockSummary);

    // Build Top-Level Window Display Target
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Game Over Screen Test");
    gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), nullptr);

    gtk_container_add(GTK_CONTAINER(window), screenInstance.getWidget());
    gtk_widget_show_all(window);

    gtk_main();
    return 0;
}

#endif // TEST_GAMEOVER_SCREEN