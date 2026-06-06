#include <gtk/gtk.h>
#include "winningHand.hpp"
#include <iostream>
#include <ostream>
#include "login.hpp"
#include "endscreen.hpp"

void set_window_background(GtkWidget *window){

    GtkCssProvider* provider = gtk_css_provider_new();
    const std::string css_data = "window { background-color: #2B4C54; }";

    gtk_css_provider_load_from_data(provider, css_data.c_str(), -1, nullptr);
    GtkStyleContext* context = gtk_widget_get_style_context(window);

    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider),
                            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}


static void print_pressed(GtkWidget *widget, gpointer data){
    std::cout << "Pressed" << std::endl;
}

static void print_lobby(GtkWidget *widget, gpointer data){
    std::cout << "lobby" << std::endl;
}

// -----------------------------------
// To show that the buttons are working
// -----------------------------------

static void activate (GtkApplication *app, gpointer user_data){
    GtkWidget* window;
    GtkWidget* title;
    GtkWidget* label;
    GtkWidget* first;
    GtkWidget* second;
    GtkWidget* third;

    // Creating main window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "End Screen");
    gtk_window_fullscreen(GTK_WINDOW(window));
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);

    // Main vertical container
    GtkWidget *mainBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), mainBox);

    // ----------------------------
    // TITLE SECTION
    // ----------------------------
    title = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    label = gtk_label_new("GAME OVER - FINAL SCORE");

    gtk_container_add(GTK_CONTAINER(title), label);

    GtkCssProvider* css_provider = gtk_css_provider_new();

    const std::string css = R"(
        label {
            font-size: 50px;
            color: #D9A036;
            font-family: "Courier New", monospace;
            font-weight: bold;
        }
        .player-card {
            background-color: #F4F1EA;
            border-radius: 25px;
            min-width: 450px;
            min-height: 80px;
            margin-bottom: 15px;
        }
        .summary {
            background-color: #F4F1EA;
            border-radius: 25px;
            min-width: 250px;
            min-height: 300px;
        }
        .play-again {
            background-image: none;
            background-color: #4A6B53;
            color: #FFFFFF;
            border: none;
            box-shadow: none;
            outline: none;
            border-radius: 15px;
            font-size: 25px;
            font-family: "Courier New", monospace;
            font-weight: bold;
        }
        .lobby {
            background-image: none;
            background-color: #222222;
            color: #FFFFFF;
            border: none;
            box-shadow: none;
            outline: none;
            border-radius: 15px;
            font-size: 25px;
            font-family: "Courier New", monospace;
            font-weight: bold;
        }
        .winner {
            font-size: 40px;
            font-family: "Courier New", monospace;
            color: #D9A036;
            font-weight: bold;
        }
        .card-area {
            background-color: #D9A036;
            border-radius: 0 25px 25px 0;
            padding: 10px;
            margin-left: 20px;
        }
        .usernames {
            color: #222222;
            font-size: 20px;
            font-weight: bold;
        }
    )";

    gtk_css_provider_load_from_data(css_provider, css.c_str(), -1, nullptr);

    GtkStyleContext* labelContext =
        gtk_widget_get_style_context(label);

    gtk_style_context_add_provider(labelContext,
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Center title horizontally
    gtk_widget_set_halign(title, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(title, 40);

    gtk_box_pack_start(GTK_BOX(mainBox), title, FALSE, FALSE, 0);

    // ----------------------------
    // CONTENT SECTION
    // ----------------------------
    GtkWidget* contentBox =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    gtk_box_pack_start(GTK_BOX(mainBox), contentBox, FALSE, FALSE, 0);

// ----------------------------
// Box for the ranking
// ---------------------------

    GtkWidget* playerListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

// ---------------------------
// First Place
// ---------------------------
    first = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    
    GtkStyleContext* firstContext =
        gtk_widget_get_style_context(first);
    gtk_style_context_add_provider(firstContext,
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_style_context_add_class(firstContext, "player-card");

    gtk_widget_set_halign(first, GTK_ALIGN_START);

// ------------------------------------
// Second Place
// ------------------------------------
    second = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

    GtkStyleContext* secondContext =
        gtk_widget_get_style_context(second);

    gtk_style_context_add_provider(secondContext,
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_style_context_add_class(secondContext, "player-card");

    gtk_widget_set_halign(second, GTK_ALIGN_START);

// -------------------------------------
// Third Place
// -------------------------------------
    third = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkStyleContext* thirdContext =
        gtk_widget_get_style_context(third);

    gtk_style_context_add_provider(thirdContext,
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_style_context_add_class(thirdContext, "player-card");

    gtk_widget_set_halign(third, GTK_ALIGN_START);

// -------------------------------------
// Assigning the names
// -------------------------------------

    auto* players = static_cast<std::vector<PlayerData>*>(user_data);

    GtkWidget* name1 = gtk_label_new(("1. " + players->at(0).name +
                    "\nCHIPS: " + std::to_string(players->at(0).chips) +
                    " " + players->at(0).bonus).c_str());
    GtkStyleContext* nameContext = 
        gtk_widget_get_style_context(name1);

    gtk_style_context_add_provider(nameContext,
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_style_context_add_class(nameContext, "usernames");
    gtk_label_set_justify(GTK_LABEL(name1), GTK_JUSTIFY_CENTER);

// -------------------- Second Place --------------------------- 
    GtkWidget* name2 = gtk_label_new(("2. " + players->at(1).name +
            "\nCHIPS: " + std::to_string(players->at(1).chips)).c_str());
    
    GtkStyleContext* name2Context =
        gtk_widget_get_style_context(name2);

    gtk_style_context_add_provider(name2Context,
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_style_context_add_class(name2Context, "usernames");
    gtk_label_set_justify(GTK_LABEL(name2), GTK_JUSTIFY_CENTER);
           
// -------------------- Third Place ---------------------------
 
    GtkWidget* name3 = gtk_label_new(("3. " + players->at(2).name +
        "\nCHIPS: " + std::to_string(players->at(2).chips)).c_str());
    
    GtkStyleContext* name3Context =
        gtk_widget_get_style_context(name3);

    gtk_style_context_add_provider(name3Context,
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_style_context_add_class(name3Context, "usernames");
    gtk_label_set_justify(GTK_LABEL(name3), GTK_JUSTIFY_CENTER);

    gtk_box_pack_start(GTK_BOX(first), name1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(second), name2, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(third), name3, TRUE, TRUE, 0);

// -------------------------------------
// Buttons
// -------------------------------------

    GtkWidget* buttonBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);

    GtkWidget* button1 = gtk_button_new_with_label("PLAY AGAIN");
    g_signal_connect(button1, "clicked", G_CALLBACK(print_pressed), nullptr);
    
    GtkStyleContext* playContext = 
        gtk_widget_get_style_context(button1);

    gtk_style_context_add_provider(playContext,
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_style_context_add_class(playContext, "play-again");    

    gtk_widget_set_halign(buttonBox, GTK_ALIGN_CENTER);

    GtkWidget* button2 = gtk_button_new_with_label("EXIT TO LOBBY");
    g_signal_connect(button2, "clicked", G_CALLBACK(print_lobby), nullptr);
   
    GtkStyleContext* lobbyContext =
        gtk_widget_get_style_context(button2);

    gtk_style_context_add_provider(lobbyContext,
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_style_context_add_class(lobbyContext, "lobby");
 
    gtk_widget_set_size_request(button1, 100, 60);
    gtk_widget_set_size_request(button2, 100, 60);

    gtk_widget_set_margin_bottom(buttonBox, 30);
    
    gtk_box_pack_start(GTK_BOX(buttonBox), button1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(buttonBox), button2, FALSE, FALSE, 0);
    
// --------------------------------
// Winner label
// --------------------------------
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget* labelWinner = gtk_label_new("WINNER");

    gtk_container_add(GTK_CONTAINER(box), labelWinner);

    GtkStyleContext* winnerContext =
        gtk_widget_get_style_context(labelWinner);
    
    gtk_style_context_add_provider(winnerContext,
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_box_pack_start(GTK_BOX(playerListBox), box, FALSE, FALSE, 10);
    gtk_style_context_add_class(winnerContext, "winner");

///////////////////////////////////////////////////////////////
    gtk_widget_set_valign(first, GTK_ALIGN_CENTER);

    gtk_widget_set_valign(second, GTK_ALIGN_CENTER);

    gtk_widget_set_valign(third, GTK_ALIGN_CENTER);
    
    gtk_widget_set_margin_top(playerListBox, 10);
    gtk_widget_set_margin_start(playerListBox, 100);

    gtk_widget_set_size_request(first, 720, 80);
    gtk_widget_set_size_request(second, 450, 80);
    gtk_widget_set_size_request(third, 450, 80);

    append_winning_hand_ui(first);

    gtk_box_pack_start(GTK_BOX(playerListBox), first, FALSE, FALSE, 30);
    gtk_box_pack_start(GTK_BOX(playerListBox), second, FALSE, FALSE, 30);
    gtk_box_pack_start(GTK_BOX(playerListBox), third, FALSE, FALSE, 10);
   
// -----------------------------------
// Money Spread
// ---------------------------------- 
    GdkPixbuf* pix = 
    gdk_pixbuf_new_from_file_at_scale("assets/Baller.png", 550, 550, TRUE, nullptr);

    GtkWidget* side_image = gtk_image_new_from_pixbuf(pix);

    GtkWidget* main_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 50); 
    gtk_widget_set_halign(main_hbox, GTK_ALIGN_CENTER);

    gtk_box_pack_start(GTK_BOX(main_hbox), playerListBox, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_hbox), side_image, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(contentBox), main_hbox, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(contentBox), buttonBox, TRUE, FALSE, 0);

    g_object_unref(pix);

// Push summary box to the right
    g_object_unref(css_provider);
    set_window_background(window);
    gtk_widget_show_all(window);
    }

int main(int argc, char* argv[])
{
  GtkApplication* app;
  int status;

    auto* playerList = get_test_players();

  app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), playerList);

  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  delete playerList;
  return status;
}
