// clientGUI.cpp : Defines the entry point for the application.
// Author: Sanaa Bebal
/* Modifications:
    06/02/26: Combining all code from beta version and my own work
*/

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <errno.h>

#include "cards.hpp"
#include "data.hpp"
#include "DataTransfer.hpp"
#include "clientGUI.hpp"

using namespace std;

typedef struct clientWindow {
    // everything that the client will be able to see and interact
    // with at a certain point within the game

    GtkWidget *mainDisplay;
    GtkWidget *playerCards;
    GtkWidget *communityCards;
    GtkWidget *callButton;
    GtkWidget *raiseButton;
    GtkWidget *foldButton;
    GtkWidget *raiseEntry;
} clientWindow;


// creating an instance of the struct

clientWindow client;
GAMESTATE officialGameState;
LOGININFO playerLoginInfo;

#define CARD_W 100
#define CARD_H 150

extern string mainDeckRef[4][13];

// ensures that community and player related components are removed

void clearContainers(GtkWidget *container){
    GList* components = gtk_container_get_children(GTK_CONTAINER(container));
    // loop to go through each components that is within a selected container
    for (GList* i = components; i != NULL; i = g_list_next(i)){
        // point to the data and destroy it
        gtk_widget_destroy(GTK_WIDGET(i -> data));
    }
    // ensure the freeing up of the memory
    g_list_free(components);
}

GtkWidget* createCard(const Card& card, bool visible){
    // check if even visible to player, if not show the back of the card only
    if (!visible){
        return createScaledImage("assets/Back.png", CARD_W, CARD_H);
    }

    // special call for Anteater card as it doesn't exist within a specific folder

    if (card.getSuit() == Anteaters || card.getValue() == Anteater){
        return createScaledImage("assets/Anteater.png", CARD_W, CARD_H);
    }

    // else return back the specific card

    return createScaledImage(mainDeckRef[card.getSuit()][card.getValue()].c_str(), CARD_W, CARD_H);
}

void updateTable() {
    // clear out anything before updating
    clearContainers(client.playerCards);
    clearContainers(client.communityCards);

    int num = playerLoginInfo.playerNum;

    if (num >= 0 && num < (int)officialGameState.allCards.size()){
        for (const auto& card: officialGameState.allCards[num]){
            // will now show the card to the player
            GtkWidget* image = createCard(card, true);
            gtk_box_pack_start(GTK_BOX(client.communityCards), image, FALSE, FALSE, 5);
        }
    }

    // human turn or not, to allow interactions on the screen

    bool humanTurn = (officialGameState.playerTurn == num);
    gtk_widget_set_sensitive(client.callButton, humanTurn);
    gtk_widget_set_sensitive(client.raiseButton, humanTurn);
    gtk_widget_set_sensitive(client.foldButton, humanTurn);

    gtk_widget_show_all(client.mainDisplay);
}

// checking the channel continuously to not interrupt the GUI

gboolean networkBuffer(gpointer inputData) {
    BUF packetBuffer(4000);
    int readBytes = read(SocketFD, packetBuffer.data(), 4000);

    // checking if there's new information
    if (readBytes < 0){
        // skipping / moving on till the next cycle 
        if (errno == EAGAIN || errno == EWOULDBLOCK){
            return TRUE;
        }
        return FALSE;
    }
    // server has been closed
    else if (readBytes == 0){
        return FALSE;
    }
    
    packetBuffer.resize(readBytes);
    officialGameState = parsingGameArguments(packetBuffer);

    // gamestate has changed therefore updates to the table must be made
    updateTable();
    return TRUE;
}

void callClicked(GtkWidget* button, gpointer inputData){

}

void foldClicked(GtkWidget* button, gpointer inputData){
    
}



// main() execution
int main(int argc, char *argv[]){
    gtk_init(argc, &argv);

    // need to establish the network SocketFD connection bc of the login

    // creating the window

    // displaying everything for the user

    // pull packets of information every 100 ms

    // giving control to loop

}
