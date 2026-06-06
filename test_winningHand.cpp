#include "winningHand.hpp"
#include <vector>
#include <random>

GtkWidget* create_poker_card(const std::string& suit, const std::string& rank) {
    std::string path = "assets/" + suit + "/" + suit+ " " + rank + ".png";
    
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(path.c_str(), nullptr);
    if (!pixbuf) {
        return gtk_label_new(rank.c_str()); 
    }

    GdkPixbuf* scaled = gdk_pixbuf_scale_simple(pixbuf, 45, 60, GDK_INTERP_BILINEAR);
    GtkWidget* image = gtk_image_new_from_pixbuf(scaled);

    g_object_unref(pixbuf);
    g_object_unref(scaled);

    return image;
}

GtkWidget* get_random_card_widget() {
    std::vector<std::string> suits = {"Clover", "Diamond", "Heart", "Spades"};
    std::vector<std::string> ranks = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"};

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> suitDist(0, suits.size() - 1);
    std::uniform_int_distribution<> rankDist(0, ranks.size() - 1);

    return create_poker_card(suits[suitDist(gen)], ranks[rankDist(gen)]);
}

void append_winning_hand_ui(GtkWidget* first_place_box) {
    // Right Section: The Card Box Container
    GtkWidget* cardBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    
    // --- FORCE ORANGE STYLES ---
    GtkCssProvider* orangeProvider = gtk_css_provider_new();
    const std::string orangeCss = R"(
        .card-area {
            background-color: #D9A036;
            border-radius: 0px 25px 25px 0px;
            padding: 10px;
            margin-left: 20px;
            min-width: 260px;
        }
    )";
    gtk_css_provider_load_from_data(orangeProvider, orangeCss.c_str(), -1, nullptr);
    
    GtkStyleContext* context = gtk_widget_get_style_context(cardBox);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(orangeProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_style_context_add_class(context, "card-area");
    g_object_unref(orangeProvider);

    
    // Populate the box container with 5 random testing card elements
    for (int i = 0; i < 5; i++) {
        GtkWidget* randomCard = get_random_card_widget();
        gtk_box_pack_start(GTK_BOX(cardBox), randomCard, FALSE, FALSE, 0);
    }

    // Attach cardBox directly to the right end edge of your player bar
    gtk_box_pack_end(GTK_BOX(first_place_box), cardBox, FALSE, FALSE, 0);
}
