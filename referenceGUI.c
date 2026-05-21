/* GUI.c : 
*  Author: Sanaa Bebal
*  Modifications: 
*  04/21/2026 SB initial modifications
*/

#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>

#include "chess.h"
#include "moves.h"
#include "rules.h"
#include "gui.h"


#ifndef G_APPLICATION_DEFAULT_FLAGS
#define G_APPLICATION_DEFAULT_FLAGS G_APPLCICATION_FLAGS_NONE
#endif

// declarations

static void startButtonClicked(GtkButton *button, gpointer userData);
static void modeButtonClicked(GtkButton *button, gpointer userData);
static void colorButtonClicked(GtkButton *button, gpointer userData);
static void levelButtonClicked(GtkButton *button, gpointer userData);
static void exitButtonClicked(GtkButton *button, gpointer userData);
static void endButtonClicked(GtkButton *button, gpointer userData);
static gboolean drawBoard(GtkWidget *widget, cairo_t *cr, gpointer userData);
static gboolean boardClicked(GtkWidget *widget, GdkEventButton *event, gpointer userData);
static void logButtonClicked(GtkButton *button, gpointer userData);
static void helpButtonClicked(GtkButton *button, gpointer userData);
static void helpBackClicked(GtkButton *button, gpointer userData);
void buildGUI(GtkApplication *app, gpointer userInput);
void scheduledRedraw();
int getPromotionPiece();
int getDifficulty();
int getGameMode();
void showMessage();
static void loadImages();
void setGameLog(MOVELIST *log);


// Module-level variables
static GtkApplication *game = NULL;
static GtkWidget *gameWindow = NULL;
static GtkWidget *container = NULL;
GtkWidget *drawArea = NULL;
static GtkWidget *endScreen = NULL;
static int lastCellSize = 0;
static gboolean allowedMoves[RANKS][FILES];
static gboolean moveValid = FALSE;

// for the function calls of showBoard and updateBoard
static PIECE *(*gameBoard)[FILES] = NULL;

static int currentPlayer = 0;
static char logFilePath[256] = "";
static MOVELIST *gameLog = NULL;
// window gone or not (don't want it pointing at random memory)
gboolean widgetGone = FALSE;

// clicker states
static gboolean Clicked = FALSE;
static int clickCount = 0;
static int selectedRow = -1;
static int selectedCol = -1;
static POS newPos;
PIECE *selectedPiece = NULL; // static hides it from mainGUI

// images
static GdkPixbuf *spriteSheet[2][7];
static GdkPixbuf *scaledSprites[2][7];
//static int cell = 40;

static const char *imagePaths[2][7] = {
    {"./assets/whiteAnt.png", // index 0 
        "./assets/whiteRook.png",
        "./assets/whiteKnight.png",
        "./assets/whiteBishop.png",
        "./assets/whiteQueen.png",
        "./assets/whiteKing.png",
        "./assets/whiteAnteater.png" // index 6
    },
    {"./assets/blackAnt.png",
      "./assets/blackRook.png",
      "./assets/blackKnight.png",
      "./assets/blackBishop.png",
      "./assets/blackQueen.png",
      "./assets/blackKing.png",
      "./assets/blackAnteater.png"
    }

};

// color scheme (for board)
#define LIGHT_R 0.9
#define LIGHT_G 0.9
#define LIGHT_B 0.9

#define DARK_R 0.36
#define DARK_G 0.24
#define DARK_B 0.12

#define SELECTED_R 0.25
#define SELECTED_G 0.70
#define SELECTED_B 0.30

// loading sprites

gboolean getMoveReady(void){
    return !Clicked;
}

POS getLastDestination(void){
    return newPos;
}

PIECE* getLastSelectedPiece(void){
    return selectedPiece;
}

static void loadImages(void) {
    for (int color = 0; color < 2; color++) {
        for (int type = 0; type < 7; type++) {
            GError *error = NULL;
            spriteSheet[color][type] = gdk_pixbuf_new_from_file(imagePaths[color][type], &error);

            // if issue loading image, print error and exit
            if (error){
                g_warning("sprite load error: %s", error->message);
                g_clear_error(&error);
                spriteSheet[color][type] = NULL;
            }
            // scaling the image to fit the given cell size
            // checking if have received NULL pointer
            /* waiting to scale until window size is known in drawBoard
            if (spriteSheet[color][type]){
                scaledSprites[color][type] = gdk_pixbuf_scale_simple(spriteSheet[color][type], cellSize, cellSize, GDK_INTERP_BILINEAR);
            }
            */
        }
    }
}

// moveValid
static void buildMove(void){
    memset(allowedMoves, 0, sizeof(allowedMoves));
    if (selectedPiece == NULL || gameBoard == NULL){
        moveValid = FALSE;
        return;
    }
    MOVE tempMove;

    for (int row = 0; row < RANKS; row++){
        for (int col = 0; col < FILES; col++){
            tempMove.pos1 = (POS){selectedRow, selectedCol};
            tempMove.pos2 = (POS){row, col};
            allowedMoves[row][col] = legalMove(&tempMove, gameBoard, gameLog, currentPlayer);
        }
    }
    moveValid = TRUE;
}

GtkApplication* initialGUI(void){

    // needs to follow a reverse DNS formating
    // setting it to NULL so it can move on without the ID
    game = gtk_application_new("org.anteater.chess", G_APPLICATION_FLAGS_NONE);
    
    // checking if it failed to create an application
    if (game == NULL){
        printf("Failed to even create the GTK Application!\n");
        return NULL;
    }

    g_signal_connect(game, "activate", G_CALLBACK(buildGUI), NULL);
    // game() should be connected from mainGUI.c
    return game;

}

static void destroyDrawArea(GtkWidget *widget, gpointer data){
    widgetGone = TRUE;
    drawArea = NULL;
}

gboolean isDrawAreaReady(void){
    return !widgetGone && drawArea != NULL && gtk_widget_get_realized(drawArea);
}

void buildGUI(GtkApplication *app, gpointer user_input){

    loadImages();

    // Window
    gameWindow = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(gameWindow), "Anteater Chess");
    gtk_window_set_default_size(GTK_WINDOW(gameWindow), 900, 760);
    gtk_window_set_resizable(GTK_WINDOW(gameWindow), TRUE);


    // Stack
    container = gtk_stack_new();
    gtk_container_add(GTK_CONTAINER(gameWindow), container);
    // can only hold one child at a time
    //gtk_container_add(GTK_CONTAINER(gameWindow), container);

    // Start Screen
    {
        GtkWidget *startBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
        gtk_widget_set_valign(startBox, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(startBox, GTK_ALIGN_CENTER);

        /*
        GtkWidget *title = gtk_label_new(GTK_ORIENTATION_VERTICAL, 20);
        gtk_widget_set_valign(startBox, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(startBox, GTK_ALIGN_CENTER);
        */

        GtkWidget *title = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(title),
                            "<span font='36' weight='bold'> Anteater Chess</span>");
        gtk_widget_set_halign(title, GTK_ALIGN_CENTER);

        GtkWidget *startButton = gtk_button_new_with_label("Start Game");
        gtk_widget_set_halign(startButton, GTK_ALIGN_CENTER);
        gtk_widget_set_size_request(startButton, 200, 55);

        // connecting the button to be seen as 
        g_signal_connect(startButton, "clicked", G_CALLBACK(startButtonClicked), NULL);

        // packing up the widgets within the box
        gtk_box_pack_start(GTK_BOX(startBox), title, FALSE, FALSE, 10);
        gtk_box_pack_start(GTK_BOX(startBox), startButton, FALSE, FALSE, 4);

        gtk_stack_add_titled(GTK_STACK(container), startBox, "start", "Start");

    }

    // Game Mode Screen
    {
        GtkWidget *modeBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
        gtk_widget_set_valign(modeBox, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(modeBox, GTK_ALIGN_CENTER);

        GtkWidget *modeLabel = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(modeLabel),
                             "<span font='22' weight='bold'>Select Game Mode</span>");
        gtk_widget_set_halign(modeLabel, GTK_ALIGN_CENTER);
        gtk_box_pack_start(GTK_BOX(modeBox), modeLabel, FALSE, FALSE, 14);

        const char *modeLabels[] = {
            "1. Player vs Computer",
            "2. Player vs Player",
            "3. Computer vs Computer"
        };

        for (int i = 0; i < 3; i++){
            GtkWidget *button = gtk_button_new_with_label(modeLabels[i]);
            gtk_widget_set_halign(button, GTK_ALIGN_CENTER);
            gtk_widget_set_size_request(button, 300, 52);
            g_object_set_data(G_OBJECT(button), "mode", GINT_TO_POINTER(i+1));
            g_signal_connect(button, "clicked", G_CALLBACK(modeButtonClicked), NULL);

            gtk_box_pack_start(GTK_BOX(modeBox), button, FALSE, FALSE, 4);
        }

        gtk_stack_add_titled(GTK_STACK(container), modeBox, "mode", "Mode");
    }

    // if P vs C, go to Computer (Difficulty) Screen
    {
        GtkWidget *difficultBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
        gtk_widget_set_valign(difficultBox, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(difficultBox, GTK_ALIGN_CENTER);

        GtkWidget *levelLabel = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(levelLabel),
                             "<span font='22' weight='bold'>Select Computer Difficulty</span>");
        gtk_widget_set_halign(levelLabel, GTK_ALIGN_CENTER);
        gtk_box_pack_start(GTK_BOX(difficultBox), levelLabel, FALSE, FALSE, 14);

        const char *levelLabels[] = {
            "Easy",
            "Medium",
            "Hard"
        };

        for (int i = 0; i < 3; i++){
            GtkWidget *button = gtk_button_new_with_label(levelLabels[i]);
            gtk_widget_set_halign(button, GTK_ALIGN_CENTER);
            gtk_widget_set_size_request(button, 300, 52);
            g_object_set_data(G_OBJECT(button), "level", GINT_TO_POINTER(i+1));
            g_signal_connect(button, "clicked", G_CALLBACK(levelButtonClicked), NULL);

            gtk_box_pack_start(GTK_BOX(difficultBox), button, FALSE, FALSE, 4);
        }

        gtk_stack_add_titled(GTK_STACK(container), difficultBox, "level", "Level");

    }

    // Color Selection Screen
    {
        GtkWidget *colorBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
        gtk_widget_set_valign(colorBox, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(colorBox, GTK_ALIGN_CENTER);

        GtkWidget *colorLabel = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(colorLabel),
                             "<span font='22' weight='bold'>Choose Your Color</span>");
        gtk_widget_set_halign(colorLabel, GTK_ALIGN_CENTER);
        
        gtk_box_pack_start(GTK_BOX(colorBox), colorLabel, FALSE, FALSE, 14);

        GtkWidget *subText = gtk_label_new("White moves first.");
        gtk_widget_set_halign(subText, GTK_ALIGN_CENTER);
        gtk_box_pack_start(GTK_BOX(colorBox), subText, FALSE, FALSE, 2);

        GtkWidget *choiceBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 24);
        gtk_widget_set_halign(choiceBox, GTK_ALIGN_CENTER);

        GtkWidget *white = gtk_button_new_with_label("White");
        gtk_widget_set_size_request(white, 160, 80);
        g_object_set_data(G_OBJECT(white), "color", GINT_TO_POINTER(0));
        g_signal_connect(white, "clicked", G_CALLBACK(colorButtonClicked), NULL);

        GtkWidget *black = gtk_button_new_with_label("Black");
        gtk_widget_set_size_request(black, 160, 80);
        g_object_set_data(G_OBJECT(black), "color", GINT_TO_POINTER(1));
        g_signal_connect(black, "clicked", G_CALLBACK(colorButtonClicked), NULL);

        GtkCssProvider *css = gtk_css_provider_new();
        gtk_css_provider_load_from_data(css,
                                        "button { background: #222; color: #fff; }", -1, NULL);
        gtk_style_context_add_provider(
            gtk_widget_get_style_context(black),
            GTK_STYLE_PROVIDER(css),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        
        g_object_unref(css);

        gtk_box_pack_start(GTK_BOX(choiceBox), white, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(choiceBox), black, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(colorBox), choiceBox, FALSE, FALSE, 16);
        
        gtk_stack_add_titled(GTK_STACK(container), colorBox, "color", "Color");
        
    }

    // Board Game Screen
    {
        GtkWidget *outside = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        
        GtkWidget *textBar = gtk_label_new("White to move.");
        gtk_widget_set_name(textBar, "textBar");
        //gtk_widget_set_name(textBar, GTK_ALIGN_CENTER);
        
        gtk_widget_set_margin_top(textBar, 8);
        gtk_widget_set_margin_bottom(textBar, 8);
        gtk_box_pack_start(GTK_BOX(outside), textBar, FALSE, FALSE, 0);
        g_object_set_data(G_OBJECT(container), "textBar", textBar);

        GtkWidget *mainDrawing = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_halign(mainDrawing, GTK_ALIGN_CENTER);
        gtk_widget_set_valign(mainDrawing, GTK_ALIGN_CENTER);
        gtk_widget_set_vexpand(mainDrawing, TRUE);

        drawArea = gtk_drawing_area_new();
        // connecting signal for the destroying drawArea
        g_signal_connect(drawArea, "destroy", G_CALLBACK(destroyDrawArea), NULL);

        gtk_widget_set_size_request(drawArea, FILES * 72, RANKS * 72);
        gtk_widget_add_events(drawArea, GDK_BUTTON_PRESS_MASK);

        g_signal_connect(drawArea, "draw", G_CALLBACK(drawBoard), NULL);
        g_signal_connect(drawArea, "button-press-event", G_CALLBACK(boardClicked), NULL);

        GtkWidget *buttonBar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

        gtk_widget_set_halign(buttonBar, GTK_ALIGN_END);
        gtk_widget_set_margin_end(buttonBar, 12);

        GtkWidget *helpButton = gtk_button_new_with_label("Help");
        GtkWidget *exitButton = gtk_button_new_with_label("Exit");

        g_signal_connect(helpButton, "clicked", G_CALLBACK(helpButtonClicked), NULL);
        g_signal_connect(exitButton, "clicked", G_CALLBACK(exitButtonClicked), NULL);

        gtk_box_pack_start(GTK_BOX(mainDrawing), drawArea, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(buttonBar), helpButton, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(buttonBar), exitButton, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(outside), buttonBar, FALSE, FALSE, 6);
        gtk_box_pack_start(GTK_BOX(outside), mainDrawing, TRUE, TRUE, 0);

        gtk_stack_add_titled(GTK_STACK(container), outside, "game", "Game");
    }

    // Help Screen (clicked on throughout game for scroll down feature)
    {
        GtkWidget *helpBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        GtkWidget *helpTitle = gtk_label_new(NULL);

        gtk_label_set_markup(GTK_LABEL(helpTitle),
                                        "<span font='28' weight='bold'>How to Play</span>");
        gtk_box_pack_start(GTK_BOX(helpBox), helpTitle, FALSE, FALSE, 10);

        GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
        gtk_widget_set_vexpand(scroll, TRUE);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

        GtkWidget *helpText = gtk_label_new(
            "Click one of your peices to select it.\n"
            "Highlighted square show the available legal moves.\n"
            "Click one of the highlighted squares to make a move.\n"
        );

        gtk_label_set_line_wrap(GTK_LABEL(helpText), TRUE);
        gtk_container_add(GTK_CONTAINER(scroll), helpText);
        gtk_box_pack_start(GTK_BOX(helpBox), scroll, TRUE, TRUE, 0);

        GtkWidget *backButton = gtk_button_new_with_label("Back to Game");
        g_signal_connect(backButton, "clicked", G_CALLBACK(helpBackClicked), NULL);
        gtk_box_pack_start(GTK_BOX(helpBox), backButton, FALSE, FALSE, 10);

        gtk_stack_add_titled(GTK_STACK(container), helpBox, "help", "Help");
    }

    // End Screen
    {
        GtkWidget *endBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
        gtk_widget_set_valign(endBox, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(endBox, GTK_ALIGN_CENTER);

        endScreen = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(endScreen),
                            "<span font='28' weight='bold'>Game Over</span>");
        gtk_widget_set_halign(endScreen, GTK_ALIGN_CENTER);

        GtkWidget *button = gtk_button_new_with_label("Return to Menu");
        gtk_widget_set_halign(button, GTK_ALIGN_CENTER);
        gtk_widget_set_size_request(button, 200, 50);

        // to log file
        GtkWidget *logButton = gtk_button_new_with_label("View Move Log");
        gtk_widget_set_halign(logButton, GTK_ALIGN_CENTER);
        gtk_widget_set_size_request(logButton, 200, 50);

        g_signal_connect(button, "clicked", G_CALLBACK(endButtonClicked), NULL);
        g_signal_connect(logButton, "clicked", G_CALLBACK(logButtonClicked), NULL);

        gtk_box_pack_start(GTK_BOX(endBox), endScreen, FALSE, FALSE, 10);
        gtk_box_pack_start(GTK_BOX(endBox), button, FALSE, FALSE, 10);
        gtk_box_pack_start(GTK_BOX(endBox), logButton, FALSE, FALSE, 10);

        gtk_stack_add_titled(GTK_STACK(container), endBox, "end", "End");
    }

    //gtk_container_add(GTK_CONTAINER(gameWindow), container);
    gtk_stack_set_visible_child_name(GTK_STACK(container), "start");
    gtk_widget_show_all(gameWindow);
    
}

void setGameLog(MOVELIST *log){
    gameLog = log;
}

static void startButtonClicked(GtkButton *button, gpointer userData) {

    // 
    g_object_set_data(G_OBJECT(game), "selectedMode", GINT_TO_POINTER(0));
    g_object_set_data(G_OBJECT(game), "selectedColor", GINT_TO_POINTER(-1));
    //gtk_stack_set_visible_child(GTK_STACK(container), GTK_WIDGET("mode"));
    gtk_stack_set_visible_child_name(GTK_STACK(container), "mode");

}

static void logButtonClicked(GtkButton *button, gpointer userData){
    FILE *fileName = fopen(logFilePath, "r");
    if (!fileName) {
        GtkWidget *err = gtk_message_dialog_new(GTK_WINDOW(gameWindow),
                        GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                        "Could not open log file: %s", logFilePath);
        gtk_dialog_run(GTK_DIALOG(err));
        gtk_widget_destroy(err);
        return;
    }

    fseek(fileName, 0, SEEK_END);
    long size = ftell(fileName);
    rewind(fileName);
    char *content = g_malloc(size + 1);
    fread(content, 1, size, fileName);
    content[size] = '\0';
    fclose(fileName);

    GtkWidget *logWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(logWindow), "Move Log");
    gtk_window_set_default_size(GTK_WINDOW(logWindow), 400, 500);
    gtk_window_set_transient_for(GTK_WINDOW(logWindow), GTK_WINDOW(gameWindow));

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    
    GtkWidget *textLog = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textLog), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textLog), GTK_WRAP_WORD);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(textLog), 10);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(textLog), 10);

    GtkTextBuffer *buffed = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textLog));
    gtk_text_buffer_set_text(buffed, content, -1);
    g_free(content);

    gtk_container_add(GTK_CONTAINER(scroll), textLog);
    gtk_container_add(GTK_CONTAINER(logWindow), scroll);
    gtk_widget_show_all(logWindow);
}

static void modeButtonClicked(GtkButton *button, gpointer userData) {

    // get the mode that was selected
    int mode = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "mode"));
    g_object_set_data(G_OBJECT(game), "selectedMode", GINT_TO_POINTER(mode));

    if (mode == 1){
        gtk_stack_set_visible_child_name(GTK_STACK(container), "level");
    }
    else if (mode == 3){
        // if computer vs computer, skip color selection and go straight to board
        g_object_set_data(G_OBJECT(game), "selectedColor", GINT_TO_POINTER(0));
        gtk_stack_set_visible_child_name(GTK_STACK(container), "game");
        //scheduledRedraw();
        startGame();
    }
    else {
        gtk_stack_set_visible_child_name(GTK_STACK(container), "color");
    }

}

static void colorButtonClicked(GtkButton *button, gpointer userData) {

    // get the color that was selected
    int color = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "color"));
    g_object_set_data(G_OBJECT(game), "selectedColor", GINT_TO_POINTER(color));

    gtk_stack_set_visible_child_name(GTK_STACK(container), "game");
    //scheduledRedraw();
    startGame();

}

static void levelButtonClicked(GtkButton *button, gpointer userData){
    // get the level that was selected
    int level = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "level"));
    g_object_set_data(G_OBJECT(game), "selectedDifficulty", GINT_TO_POINTER(level));

    gtk_stack_set_visible_child_name(GTK_STACK(container), "color"); // used to be game
    scheduledRedraw();
}

static void endButtonClicked(GtkButton *button, gpointer userData){
    g_object_set_data(G_OBJECT(game), "selectedMode", GINT_TO_POINTER(0));
    g_object_set_data(G_OBJECT(game), "selectedColor", GINT_TO_POINTER(-1));
    g_object_set_data(G_OBJECT(game), "selectedDifficulty", GINT_TO_POINTER(0));

    gtk_stack_set_visible_child_name(GTK_STACK(container), "start");
}

static void helpButtonClicked(GtkButton *button, gpointer userData){
    gtk_stack_set_visible_child_name(GTK_STACK(container), "help");
}

static void helpBackClicked(GtkButton *button, gpointer userData){
    gtk_stack_set_visible_child_name(GTK_STACK(container), "game");
}

static void exitButtonClicked(GtkButton *button, gpointer userData){
    showMessage("You exited the game.");
}

void resetClickState(void){
    //gameBoard = NULL;
    Clicked = TRUE;
    clickCount = 0;
    selectedPiece = NULL;
    selectedRow = -1;
    selectedCol = -1;
    moveValid = FALSE;
    memset(allowedMoves, 0, sizeof(allowedMoves));
}

static gboolean boardClicked(GtkWidget *widget, GdkEventButton *event, gpointer user_data){
    // should be left button clicking the board, ignore otherwise
    if (event -> button != 1){
        return FALSE;
    }
    if (game == NULL) {
        return FALSE;
    }

    if (!Clicked) {
        return FALSE;
    }

    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);
    double cellWidth = (double)width / FILES;
    double cellHeight = (double)height / RANKS;

    double cell = (cellWidth < cellHeight) ? cellWidth : cellHeight;

    double xOffset = (width - cell * FILES) / 2.0;
    double yOffset = (height - cell * RANKS) / 2.0;
    
    int column = (int)((event -> x - xOffset) / cell);
    int display_row = (int)((event -> y - yOffset) / cell);
    int row = (RANKS - 1) - display_row;


    // outside the parameters of the board
    if (column < 0 || column >= FILES || row < 0 || row >= RANKS){
        return FALSE;
    }

    if (clickCount == 0){
        if (gameBoard[row][column] == NULL){
            return FALSE;
        }
        if (gameBoard[row][column] -> PieceColor != currentPlayer){
            return FALSE;
        }
        selectedPiece = gameBoard[row][column];
        selectedRow = row;
        selectedCol = column;
        clickCount = 1;
        //buildMove(); // building possible moves
        scheduledRedraw();
        return TRUE;
    }
    else {
        // empty or enemy
        newPos.x = row;
        newPos.y = column;
        clickCount = 0;
        // resets the highlight
        selectedRow = -1;
        selectedCol = -1;
        moveValid = FALSE;
        Clicked = FALSE; // breaks the while loop
        // moving redraw until after clicks one and two happen
        //buildMove();
        scheduledRedraw();
        return TRUE;
    }
}

void setLogFilePath(const char *path){
    snprintf(logFilePath, sizeof(logFilePath), "%s", path);
}

static gboolean drawBoard(GtkWidget *widget, cairo_t *cr, gpointer userData) {
    // protection

    if (gameBoard == NULL){
        return FALSE;
    }
    
    printf("drawBoard is called, gameBoard=%p\n", (void *)gameBoard);
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);


    double cell = (width / FILES < height / RANKS) ? (width / FILES) : (height / RANKS);

    double xOffset = (width - cell * FILES) / 2.0;
    double yOffset = (height - cell * RANKS) / 2.0;


    // Scaling Block
    if ((int)cell != lastCellSize && (int)cell > 0){
        int spriteSize = (int)cell - 4;
        if (spriteSize < 1) spriteSize = 1;

        for (int color = 0; color < 2; color++){
            for (int type = 0; type < 7; type++){
                if (scaledSprites[color][type] != NULL){
                    g_object_unref(scaledSprites[color][type]);
                    scaledSprites[color][type] = NULL;
                }
                if (spriteSheet[color][type]){
                    scaledSprites[color][type] = gdk_pixbuf_scale_simple(spriteSheet[color][type],
                                            spriteSize, spriteSize, GDK_INTERP_BILINEAR);
                }
            }
        }
        lastCellSize = (int)cell;
    }

    // only have to build once
    if (clickCount == 1 && selectedPiece != NULL && !moveValid){
        buildMove();
    }

    for (int row = 0; row < RANKS; row++){
        // fixing it so that it prints out 8 to 1
        int board_row = (RANKS - 1) - row;
        for (int col = 0; col < FILES; col++){
            double x = xOffset + col * cell;
            double y = yOffset + board_row * cell;

            if (row == selectedRow && col == selectedCol && clickCount == 1){
                cairo_set_source_rgb(cr, SELECTED_R, SELECTED_G, SELECTED_B);
            }
            else if (clickCount == 1 && selectedPiece != NULL){
                if (!moveValid){
                    //buildMove();
                }
                if (allowedMoves[row][col]){
                    cairo_set_source_rgba(cr, 1.0,1.0, 0.0, 0.3);
                } else {
                    ((row + col) % 2 == 0) ? cairo_set_source_rgb(cr, LIGHT_R, LIGHT_G, LIGHT_B) : 
                                            cairo_set_source_rgb(cr, DARK_R, DARK_G, DARK_B);
                }
            }
            else {
                // cairo_set_source_rgb(cr, DARK_R, DARK_G, DARK_B);
                ((row + col) % 2 == 0) ? cairo_set_source_rgb(cr, LIGHT_R, LIGHT_G, LIGHT_B):
                                        cairo_set_source_rgb(cr, DARK_R, DARK_G, DARK_B);
            }

            cairo_rectangle(cr, x, y, cell, cell);
            cairo_fill(cr);
            
            // drawing pieces onto the board
            if (gameBoard != NULL){
                PIECE *p = gameBoard[row][col];
                if (p != NULL){
                    int pColor = p ->PieceColor;
                    int pType = p -> PieceType;

                    if (pColor >= 0 && pColor <= 1 && pType >= 0 && pType <= 6){
                        GdkPixbuf *pb = scaledSprites[p->PieceColor][p->PieceType];
                        
                        if (pb) {
                            // centering the image
                            gdk_cairo_set_source_pixbuf(cr, pb, x + 2, y + 2 );
                            cairo_paint(cr);
                        }
                    }
                }
            }
        }
    }

    cairo_select_font_face(cr, "Sans",
                            CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    
    cairo_set_font_size(cr, cell * 0.18);

    for (int row = 0; row < RANKS; row++) {
        int board_row = (RANKS - 1) - row;
        double y = yOffset + row * cell + cell * 0.55;
        cairo_move_to(cr, xOffset + 3, y);
        if (row % 2 == 0){
            cairo_set_source_rgba(cr, DARK_R, DARK_G, DARK_B, 0.7);
        }
        else{
            cairo_set_source_rgba(cr, LIGHT_R, LIGHT_G, LIGHT_B, 0.7);
        }
        char buf[4];
        // fix here to get the numbers for the columns
        snprintf(buf, sizeof(buf), "%d", board_row + 1);
        cairo_show_text(cr, buf);
    }

    
    return FALSE;

}




void showBoard(PIECE *board[RANKS][FILES]){

    gameBoard = board;
    scheduledRedraw();

}

void setCurrentPlayer(int playerNum){
    currentPlayer = playerNum;
}

void update_board(PIECE *board[RANKS][FILES], PIECE *piece){

    (void)piece;
    printf("update_board called, gameBoard should refresh");
    gameBoard = board;
    scheduledRedraw();
}

gboolean isGUIActive(void){
    // checking if gamewindow and window is still there
    return !widgetGone && gameWindow != NULL;
}

void scheduledRedraw(void){

    printf("scheduledRedraw: widgetGone=%d drawArea=%p realized=%d\n",
        widgetGone, (void*)drawArea, drawArea ? gtk_widget_get_realized(drawArea) : -1);

    if(!widgetGone && drawArea != NULL && gtk_widget_get_realized(drawArea)){
        gtk_widget_queue_draw(drawArea);
    }
    else {
        printf("scheduledRedraw: SKIPPED OVER");
    }

}

int getPromotionPiece(void){
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Pawn Promotion", GTK_WINDOW(gameWindow),
        GTK_DIALOG_MODAL,
        "OK", GTK_RESPONSE_OK, NULL
    );

    GtkWidget *options = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(options), "Queen");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(options), "Rook");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(options), "Bishop");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(options), "Anteater");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(options), "Knight");
    
    gtk_combo_box_set_active(GTK_COMBO_BOX(options), 0);

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_add(GTK_CONTAINER(content), options);
    gtk_widget_show_all(dialog);

    gtk_dialog_run(GTK_DIALOG(dialog));
    int choice = gtk_combo_box_get_active(GTK_COMBO_BOX(options));
    gtk_widget_destroy(dialog);
    return choice; // 0 = Queen, 1 = Rook, 2 = Bishop, 3 = Knight

}

//NEED TO FIX THIS
int getAnteatingChoice(void){
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Continue Anteating?", GTK_WINDOW(gameWindow),
        GTK_DIALOG_MODAL,
        "YES", GTK_RESPONSE_YES, 
        "NO", GTK_RESPONSE_NO,
        NULL
    );

    GtkWidget *prompter = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget *label = gtk_label_new(
        "Do you want your Anteater to eat the Ant?"
    );

    gtk_widget_set_margin_top(label, 12);
    gtk_widget_set_margin_bottom(label, 12);
    gtk_widget_set_margin_start(label, 16);
    gtk_widget_set_margin_end(label, 16);
    gtk_container_add(GTK_CONTAINER(prompter), label);
    gtk_widget_show_all(dialog);

    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    return (response == GTK_RESPONSE_YES) ? 1 : 0;
}

int getDifficulty(void){
    if(!game){
        return 1;
    }
    return GPOINTER_TO_INT(g_object_get_data(G_OBJECT(game), "selectedDifficulty"));
}

int getUserMove(PIECE *board[RANKS][FILES], PIECE **piece, POS *new_pos){
    if (!drawArea) {
        return -1;
    }
    gameBoard = board;
    Clicked = TRUE;
    clickCount = 0;
    selectedPiece = NULL;
    selectedRow = -1;
    selectedCol = -1;

    scheduledRedraw();

    while (Clicked){
        //gtk_main_iteration();
        if (!g_main_context_iteration(NULL, TRUE)){
            return -1;
        }
    }

    // check if selectedPiece isn't NULL
    if (selectedPiece == NULL) {
        return -1;
    }

    if (piece) *piece = selectedPiece;
    if (new_pos) *new_pos = newPos;

    return 1;

}

// similar logic to getSelectedColor
int getGameMode(void){
    if (!game){
        return 0;
    }
    return GPOINTER_TO_INT(g_object_get_data(G_OBJECT(game), "selectedMode"));
}


int getPlayerColor(void){
    // failed
    if (!game){
        return -1;
    }
    // will return 0 for White, 1 for Black, -1 if anything different
    return GPOINTER_TO_INT(g_object_get_data(G_OBJECT(game), "selectedColor"));
}

// supposed to switch to the end screen telling the following:
/* 1- Player won the game (Player vs Computer)
*  2- Player 1 or 2 won the game (Player vs Player or Computer vs Computer)
*  3- Game is a draw
*  4- User exited the game early
* Prompt them to exit and go back to the home screen
*/

void showMessage(const char *message){
    if(endScreen){
        char finalMessage[512];
        // where the final outcome of the game is displayed
        snprintf(finalMessage, sizeof(finalMessage),
                "<span font ='26' weight='bold'>%s</span>", message);
        gtk_label_set_markup(GTK_LABEL(endScreen), finalMessage);
            
    }
    if (container){
        gtk_stack_set_visible_child_name(GTK_STACK(container), "end");
    }

}


void closeGUI(void){
    for (int color = 0; color < 2; color++){
        for (int type = 0; type < 7; type++){
            if (spriteSheet[color][type]){
                g_object_unref(spriteSheet[color][type]);
                spriteSheet[color][type] = NULL;
            }
            if(scaledSprites[color][type]){
                g_object_unref(scaledSprites[color][type]);
                scaledSprites[color][type] = NULL;
            }
        }
    }

    if (gameWindow){
        gtk_widget_destroy(gameWindow);
    }
    if (game){
        g_application_quit(G_APPLICATION(game));
    }

}

// NEED TO IMPLEMENT:
/* highlighting all the possible pos
 * function returning a MOVE after prompting user (for MakeMove, pawnPromotion and antEat)
*/