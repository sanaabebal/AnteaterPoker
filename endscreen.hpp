#ifndef ENDSCREEN_HPP
#define ENDSCREEN_HPP

#include <gtk/gtk.h>
#include <iostream>
#include <ostream>

void set_window_background(GtkWidget *window);

static void print_pressed(GtkWidget *widget, gpointer data);

static void print_lobby(GtkWidget *widget, gpointer data);

static void activate (GtkApplication *app, gpointer user_data);

int main(int argc, char* argv[]);

#endif
