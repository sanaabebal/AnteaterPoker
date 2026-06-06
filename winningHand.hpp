#ifndef WINNING_HAND_HPP
#define WINNING_HAND_HPP

#include <gtk/gtk.h>
#include <string>

// Function declarations
GtkWidget* create_poker_card(const std::string& suit, const std::string& rank);
GtkWidget* get_random_card_widget();
void append_winning_hand_ui(GtkWidget* first_place_box);

#endif // WINNING_HAND_H
