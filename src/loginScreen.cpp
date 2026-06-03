#include <gtk/gtk.h>
#include <string.h>

const gchar *loginCSS =
    "window {}"
    ".main-title {}"
    ".subtitle {}"
    ".fild-label {}"
    "entry, combo {}"
    ".action-button label {}"
    ".action-button:hover {}"
;

static void joinClicked(GtkWidget *widget, gpointer inputData) {
    g_print("Parsing credentionals and moving onto waiting room...\n");
    gtk_main_quit();
}

int main(int argc, char *argv[]){
    gtk_init(&argc, &argv);
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Anteater Poker - Login");
    gtk_window_set_default_size(GTK_WINDOW(window), 420, 560);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, lobby_css, -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider), 
                                                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_set_border_width(GTK_CONTAINER(main_box), 32);
    gtk_container_add(GTK_CONTAINER(window), main_box);


    
}