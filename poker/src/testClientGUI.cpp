/* testClientGUIW9.cpp */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <gtk/gtk.h>
#include <assert.h>

#include "cards.hpp"
#include "data.hpp"
#include "DataTransfer.hpp"

/**************** Global vars *********************/
LOGININFO playerLoginInfo;
GAMESTATE logGameState;


/******** GUI Functions **************************************************/
GtkWidget *Window;
GtkWidget *MainDisplay;

typedef struct gamewindow{
    GtkWidget *logScrollBox;
        GtkWidget *logEvents;
        GtkTextBuffer *logEventsBuffer;
    

    GtkWidget *playerIconsBox;
        GtkWidget *playerIcons[10];

    GtkWidget *cardsBox;
        GtkWidget *playerCardsBox;
            GtkWidget *playerCards[2];
        GtkWidget *commCardsBox;
            GtkWidget *commCards[5];

    GtkWidget *playerInputBox;
        GtkWidget *callButton;
        GtkWidget *raiseBox;
            GtkWidget *raiseButton;
            GtkWidget *raiseEntry;
        GtkWidget *foldButton;
        GtkWidget *InputError;
        GtkWidget *helpMenuButton;
        GtkWidget *ShutdownButton;


    GtkWidget *playersAndCardsBox;
    GtkWidget *movesAndLogBox;

} GameWindow;
GameWindow gameWindow;

std::string mainDeckRef[4][13] = {
    {"assets/Clovers/Clover 2.png", "assets/Clovers/Clover 3.png", "assets/Clovers/Clover 4.png", "assets/Clovers/Clover 5.png", 
        "assets/Clovers/Clover 6.png", "assets/Clovers/Clover 7.png", "assets/Clovers/Clover 8.png", "assets/Clovers/Clover 9.png", "assets/Clovers/Clover 10.png", 
        "assets/Clovers/Clover J.png", "assets/Clovers/Q Clover.png", "assets/Clovers/Clover K.png", "assets/Clovers/Clover A.png"},
    {"assets/Hearts/Heart 2.png", "assets/Hearts/Heart 3.png", "assets/Hearts/Heart 4.png", "assets/Hearts/Heart 5.png", 
        "assets/Hearts/Heart 6.png", "assets/Hearts/Heart 7.png", "assets/Hearts/Heart 8.png", "assets/Hearts/Heart 9.png", "assets/Hearts/Heart 10.png", 
        "assets/Hearts/Heart J.png", "assets/Hearts/Heart Q.png", "assets/Hearts/Heart K.png", "assets/Hearts/Heart A.png"},
    {"assets/Diamonds/Diamond 2.png", "assets/Diamonds/Diamond 3.png", "assets/Diamonds/Diamond 4.png", "assets/Diamonds/Diamond 5.png", 
        "assets/Diamonds/Diamond 6.png", "assets/Diamonds/Diamond 7.png", "assets/Diamonds/Diamond 8.png", "assets/Diamonds/Diamond 9.png", "assets/Diamonds/Diamond 10.png", 
        "assets/Diamonds/Diamond J.png", "assets/Diamonds/Diamond Q.png", "assets/Diamonds/Diamond K.png", "assets/Diamonds/Diamond A.png"},
    {"assets/Spades/Spade 2.png", "assets/Spades/Spade 3.png", "assets/Spades/Spade 4.png", "assets/Spades/Spade 5.png", 
        "assets/Spades/Spade 6.png", "assets/Spades/Spade 7.png", "assets/Spades/Spade 8.png", "assets/Spades/Spade 9.png", "assets/Spades/Spade 10.png", 
        "assets/Spades/Spade J.png", "assets/Spades/Spade Q.png", "assets/Spades/Spade K.png", "assets/Spades/Spade A.png"}
};

std::string anteaterRef = "assets/Anteater.png";
std::string backRef = "assets/Back.png";




void InitGameScreenWindow(GAMESTATE &gameState, int playerNum);

void resetMainDisplay(){
    GList *children = gtk_container_get_children(GTK_CONTAINER(MainDisplay)); // returns a doubly-linked list of pointers to children GtkWidgets
    GList *i;

    for(i = children; i != NULL; i = g_list_next(i)){ // looping through the list
        gtk_container_remove(GTK_CONTAINER(MainDisplay), GTK_WIDGET(i->data));
    }

    // Wrapup:  Freeing list pointer
    g_list_free(children);
}


void defaultButtonClick(GtkWidget *button, gpointer clickData){
    printf("A button was clicked!\n");
}
void InitGameScreenWindow(GAMESTATE &gameState, int playerNum){
    resetMainDisplay();

    /*   Creating entries   */
    // Player entries
        char description[500];    
        for(unsigned int i=0; i<gameState.players.size(); i++){
            PLAYER player = gameState.players[i];
            sprintf(description, "Player %d: %s\nPoints: %d\nBet: %d\nIn round: %s\nEliminated?: %s\n%s\n", i+1, player.name, player.score, player.bet, 
                (player.isInHand) ? "yes" : "no", (player.isEliminated) ? "yes" : "no", (i == gameState.dealerPlayer) ? "DEALER" : "");
            gameWindow.playerIcons[i] = gtk_label_new(description);
        }

    // Card entries
        if((unsigned int)playerNum >= gameState.allCards.size()){
            printf("ERROR:  When trying to display cards for a given player, the player number exceeded the size of the array.  Exiting function...\n");
            return;
        }
        std::string cardStr;
        Card foundCard;
        for(int i=0; i<2; i++){
            foundCard = gameState.allCards[playerNum][i];
            cardStr = mainDeckRef[foundCard.getSuit()][foundCard.getValue()];
            gameWindow.playerCards[i] = gtk_image_new_from_file(cardStr.c_str());
        }

        if(gameState.allCards.size() < 3){
            printf("ERROR:  When trying to display community cards, noticed that there are only two piles.  This is not enough.  Exiting function...\n");
            return;
        }
        for(int i=0; i<5; i++){
            int r = gameState.round;
            if(  r == Preflop || (r == Flop && i>2) || (r == Turn && i>3)  ){
                gameWindow.commCards[i] = gtk_image_new_from_file(backRef.c_str());
            } else{
                foundCard = gameState.allCards[gameState.allCards.size()-2][i]; // ex:  player1, player2, player3, _commCards_, restOfCards
                cardStr = mainDeckRef[foundCard.getSuit()][foundCard.getValue()];
                gameWindow.commCards[i] = gtk_image_new_from_file(cardStr.c_str());
            }
        }
    
    // Player input entries
        gameWindow.callButton = gtk_button_new_with_label("Call");
        gameWindow.raiseButton = gtk_button_new_with_label("Confirm Raise");
        gameWindow.raiseEntry = gtk_entry_new();
        gameWindow.foldButton = gtk_button_new_with_label("Fold");
        gameWindow.InputError = gtk_label_new("Illegal move.  Please try again.");
        gameWindow.helpMenuButton = gtk_button_new_with_label("Help Menu");
        gameWindow.ShutdownButton = gtk_button_new_with_label("End game and stop server");

    // Logging
        gameWindow.logEvents = gtk_text_view_new();
        gameWindow.logEventsBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(gameWindow.logEvents));
        gtk_text_view_set_editable(GTK_TEXT_VIEW(gameWindow.logEvents), FALSE);
        gameWindow.logScrollBox = gtk_scrolled_window_new(NULL, NULL);

    
    /*  Creating extra boxes; Staging  */ 
    // Boxes
        // Main Display
        MainDisplay = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        gtk_container_add(GTK_CONTAINER(Window), MainDisplay);

        // Overarching boxes
        gameWindow.playersAndCardsBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        gtk_container_add(GTK_CONTAINER(MainDisplay), gameWindow.playersAndCardsBox);
        gameWindow.movesAndLogBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        gtk_container_add(GTK_CONTAINER(MainDisplay), gameWindow.movesAndLogBox);

        // Players
        gameWindow.playerIconsBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        gtk_container_add(GTK_CONTAINER(gameWindow.playersAndCardsBox), gameWindow.playerIconsBox);
        for(unsigned int i=0; i<gameState.players.size(); i++){
            gtk_container_add(GTK_CONTAINER(gameWindow.playerIconsBox), gameWindow.playerIcons[i]);
        }

        // CardBox
        gameWindow.cardsBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        gtk_container_add(GTK_CONTAINER(gameWindow.playersAndCardsBox), gameWindow.cardsBox);
        // CommCards
        gameWindow.commCardsBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        gtk_container_add(GTK_CONTAINER(gameWindow.cardsBox), gameWindow.commCardsBox);
        for(int i=0; i<5; i++){
            gtk_container_add(GTK_CONTAINER(gameWindow.commCardsBox), gameWindow.commCards[i]);
        }
        // PlayerCards
        gameWindow.playerCardsBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        gtk_container_add(GTK_CONTAINER(gameWindow.cardsBox), gameWindow.playerCardsBox);
        for(int i=0; i<2; i++){
            gtk_container_add(GTK_CONTAINER(gameWindow.playerCardsBox), gameWindow.playerCards[i]);
        }


        // Moves
        gameWindow.playerInputBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        gtk_container_add(GTK_CONTAINER(gameWindow.movesAndLogBox), gameWindow.playerInputBox);
        
        gtk_container_add(GTK_CONTAINER(gameWindow.playerInputBox), gameWindow.callButton);
        gameWindow.raiseBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        gtk_container_add(GTK_CONTAINER(gameWindow.playerInputBox), gameWindow.raiseBox);
        gtk_container_add(GTK_CONTAINER(gameWindow.raiseBox), gameWindow.raiseEntry);
        gtk_container_add(GTK_CONTAINER(gameWindow.raiseBox), gameWindow.raiseButton);
        gtk_container_add(GTK_CONTAINER(gameWindow.playerInputBox), gameWindow.foldButton);
        // gtk_container_add(GTK_CONTAINER(gameWindow.playerInputBox), gameWindow.InputError); -- Don't want to display this unless there is an actual input error
        gtk_container_add(GTK_CONTAINER(gameWindow.playerInputBox), gameWindow.helpMenuButton);
        gtk_container_add(GTK_CONTAINER(gameWindow.playerInputBox), gameWindow.ShutdownButton);


        // Logs
        // gameWindow.logScrollBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        gtk_container_add(GTK_CONTAINER(gameWindow.movesAndLogBox), gameWindow.logScrollBox);

        // Moves (and raiseBox)
        gameWindow.raiseBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        
        


    /*   Activating buttons   */
        g_signal_connect(gameWindow.callButton, "clicked", G_CALLBACK(defaultButtonClick), NULL);
        g_signal_connect(gameWindow.raiseButton, "clicked", G_CALLBACK(defaultButtonClick), NULL);
        g_signal_connect(gameWindow.foldButton, "clicked", G_CALLBACK(defaultButtonClick), NULL);
        g_signal_connect(gameWindow.helpMenuButton, "clicked", G_CALLBACK(defaultButtonClick), NULL);
        g_signal_connect(gameWindow.ShutdownButton, "clicked", G_CALLBACK(defaultButtonClick), NULL);


    // Wrapup
    gtk_widget_show_all(Window);
}

int main(int argc, char *argv[]){
    /* initialize the GTK libraries */
    gtk_init(&argc, &argv);

    /* create the main, top level window */
    Window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(Window), "Poker Client View");
    gtk_window_set_default_size(GTK_WINDOW(Window), 260, 280);
    gtk_container_set_border_width (GTK_CONTAINER(Window), 10);

    g_signal_connect(Window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    /* overall vertical arrangement in the window -- now done through Init function */
    // MainDisplay = gtk_vbox_new(FALSE, 10);
    // MainDisplay = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    // gtk_container_add(GTK_CONTAINER(Window), MainDisplay);

    #ifndef TESTING
        // Setting up stuff (LOGININFO)
        playerLoginInfo.isHost = 1;
        playerLoginInfo.numPlayers = 4;
        playerLoginInfo.playerNum = 0;
        playerLoginInfo.playerType = Human;
        strcpy(playerLoginInfo.playerName, "Lauren");

        // Setting up stuff (GAMESTATE)
        PILE player1Cards = {{Ace, Hearts}, {Two, Spades}};
        PILE player2Cards = {{Two, Clubs}, {Ten, Diamonds}};
        PILE player3Cards = {{Anteater, Anteaters}, {Queen, Spades}};
        // PILE player4Cards = {{King, Hearts}, {Ace, Spades}}; simulating eliminated player

        PILE commCards = {{Three, Spades}, {Four, Spades}, {Five, Spades}, {Six, Spades}};
        PILE restOfCards = {{Anteater, Anteaters}}; // not actually really needed...so creating dummy vector

        logGameState.allCards = {player1Cards, player2Cards, player3Cards, commCards, restOfCards};
        logGameState.callAmount = 20;
        logGameState.dealerPlayer = 1;
        logGameState.numPlayers = 4;


        PLAYER player1, player2, player3, player4;
        player1.playerNum = 0; player1.bet = 5; player1.isEliminated = 0; player1.isInHand = 1; strcpy(player1.name, "RandomPerson1"); 
            player1.score = 100; player1.playerType = Human;
        player2.playerNum = 1; player2.bet = 6; player2.isEliminated = 0; player2.isInHand = 1; strcpy(player2.name, "RandomPerson2"); 
            player2.score = 101; player2.playerType = Computer;
        player3.playerNum = 2; player3.bet = 4; player3.isEliminated = 0; player3.isInHand = 0; strcpy(player3.name, "RandomPerson3"); 
            player3.score = 150; player3.playerType = Computer;
        player4.playerNum = 3; player4.bet = 0; player4.isEliminated = 0; player4.isInHand = 0; strcpy(player4.name, "RandomPerson4"); 
            player4.score = 0; player4.playerType = Human;
        
        logGameState.players = {player1, player2, player3, player4};

        logGameState.playerTurn = 0;
        logGameState.pot = 20;
        logGameState.round = Flop;


    #endif

    // Testing display
    for(int i=0; i<4; i++){
        InitGameScreenWindow(logGameState, i);

        gtk_widget_show_all(Window);
        gtk_main();
    }
}

