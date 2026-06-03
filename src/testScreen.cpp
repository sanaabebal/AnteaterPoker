#include "testScreen.hpp"
#include <cstdio>

GameOverScreen::GameOverScreen() {

    // MAIN CONTAINER
    mainBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);

    gtk_widget_set_margin_top(mainBox, 30);
    gtk_widget_set_margin_bottom(mainBox, 30);
    gtk_widget_set_margin_start(mainBox, 30);
    gtk_widget_set_margin_end(mainBox, 30);

    // TITLE
    GtkWidget* title =
        gtk_label_new("GAME OVER - FINAL SCORES");

    gtk_box_pack_start(
        GTK_BOX(mainBox),
        title,
        FALSE,
        FALSE,
        10
    );

    // CENTER AREA
    GtkWidget* centerBox =
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 30);

    // =========================
    // LEFT SIDE PLAYER LIST
    // =========================

    GtkWidget* playerBox =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);

    GtkWidget* p1 =
        createPlayerRow(
            "1.",
            "ACE_PRO_99",
            "CHIPS: 1,450,200 (+$4,250)"
        );

    GtkWidget* p2 =
        createPlayerRow(
            "2.",
            "PLAYER_TWO",
            "CHIPS: 1,450,200"
        );

    GtkWidget* p3 =
        createPlayerRow(
            "3.",
            "PLAYER_THREE",
            "CHIPS: 1,450,200"
        );

    gtk_box_pack_start(GTK_BOX(playerBox), p1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(playerBox), p2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(playerBox), p3, FALSE, FALSE, 0);

    // =========================
    // CENTER MONEY DISPLAY
    // =========================

    GtkWidget* moneyLabel =
        gtk_label_new("💰\nPOKER POT");

    gtk_box_pack_start(
        GTK_BOX(centerBox),
        playerBox,
        FALSE,
        FALSE,
        10
    );

    gtk_box_pack_start(
        GTK_BOX(centerBox),
        moneyLabel,
        FALSE,
        FALSE,
        50
    );

    // =========================
    // RIGHT SESSION SUMMARY
    // =========================

    GtkWidget* summaryFrame =
        gtk_frame_new(NULL);

    gtk_widget_set_size_request(
        summaryFrame,
        220,
        220
    );

    GtkWidget* summaryBox =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    GtkWidget* summaryTitle =
        gtk_label_new("SESSION SUMMARY");

    GtkWidget* summaryStats =
        gtk_label_new(
            "Total Hands: 142\n\n"
            "Duration: 2:38:15\n\n"
            "Biggest Pot:\n"
            "750,000"
        );

    gtk_box_pack_start(
        GTK_BOX(summaryBox),
        summaryTitle,
        FALSE,
        FALSE,
        10
    );

    gtk_box_pack_start(
        GTK_BOX(summaryBox),
        summaryStats,
        FALSE,
        FALSE,
        10
    );

    gtk_container_add(
        GTK_CONTAINER(summaryFrame),
        summaryBox
    );

    gtk_box_pack_start(
        GTK_BOX(centerBox),
        summaryFrame,
        FALSE,
        FALSE,
        10
    );

    // ADD CENTER AREA
    gtk_box_pack_start(
        GTK_BOX(mainBox),
        centerBox,
        TRUE,
        TRUE,
        10
    );

    // =========================
    // BUTTONS
    // =========================

    GtkWidget* buttonBox =
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);

    GtkWidget* playAgainButton =
        gtk_button_new_with_label("PLAY AGAIN");

    GtkWidget* exitButton =
        gtk_button_new_with_label("EXIT TO LOBBY");

    gtk_box_pack_start(
        GTK_BOX(buttonBox),
        playAgainButton,
        FALSE,
        FALSE,
        0
    );

    gtk_box_pack_start(
        GTK_BOX(buttonBox),
        exitButton,
        FALSE,
        FALSE,
        0
    );

    gtk_box_pack_start(
        GTK_BOX(mainBox),
        buttonBox,
        FALSE,
        FALSE,
        10
    );
}

GtkWidget* GameOverScreen::createPlayerRow(
    const char* rank,
    const char* playerName,
    const char* chipText
) {

    GtkWidget* frame =
        gtk_frame_new(NULL);

    gtk_widget_set_size_request(
        frame,
        300,
        70
    );

    GtkWidget* box =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    char nameBuffer[256];

    snprintf(
        nameBuffer,
        sizeof(nameBuffer),
        "%s %s",
        rank,
        playerName
    );

    GtkWidget* playerLabel =
        gtk_label_new(nameBuffer);

    GtkWidget* chipLabel =
        gtk_label_new(chipText);

    gtk_box_pack_start(
        GTK_BOX(box),
        playerLabel,
        FALSE,
        FALSE,
        5
    );

    gtk_box_pack_start(
        GTK_BOX(box),
        chipLabel,
        FALSE,
        FALSE,
        5
    );

    gtk_container_add(
        GTK_CONTAINER(frame),
        box
    );

    return frame;
}

GtkWidget* GameOverScreen::getWidget() {
    return mainBox;
}