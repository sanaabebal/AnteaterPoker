/* clientGUIW9.cpp - BETA */

/* GTK_ClockClient.c: GUI example interactive TCP/IP client for ClockServer
 * Author: Rainer Doemer, 04/22/22 (based on simple ClockClient.c)
 */

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


#define DEFAULT_SERVERPORT 10080
#define DEFAULT_SERVERNAME "bondi"


/* #define DEBUG */	/* be verbose */


/*** global variables ****************************************************/

const char *Program	/* program name for descriptive diagnostics */
	= NULL;
struct sockaddr_in
	ServerAddress;	/* server address we connect with */
int SocketFD = 0; // used by several programs

GAMESTATE officialGameState;
LOGININFO playerLoginInfo;
int officialPlayerSocket = 0;


/*** global functions ****************************************************/

void FatalError(		/* print error diagnostics and abort */
	const char *ErrorMsg)
{
    fputs(Program, stderr);
    fputs(": ", stderr);
    perror(ErrorMsg);
    fputs(Program, stderr);
    fputs(": Exiting!\n", stderr);
    exit(20);
} /* end of FatalError */

LOGININFO Talk2ServerLogin(LOGININFO loginInfo){	/* communicate with the server */
    int n;

    /* Creating buffer, sending it to server */
    BUF RecvBuf(2000); /* message buffer for receiving a message -- think the max for login and game is well below 2000 bytes, but using this just in case */
    BUF SendBuf; /* message buffer for sending a response */

    SendBuf = createBuffer(loginInfo);
    n = write(SocketFD, SendBuf.data(), SendBuf.size());
    if (n < 0){   
        FatalError("writing to socket failed");
    }
    
    /* Getting response from server and closing the socket */
    n = read(SocketFD, RecvBuf.data(), 2000);
    if (n < 0) {   
        FatalError("reading from socket failed");
    }
    RecvBuf.resize(n);

    // Wrapup:  Getting new structure and returning it
    loginInfo = parsingLoginArguments(RecvBuf);
    return loginInfo;
} /* end of Talk2Server */

LOGININFO SendServerLogin(LOGININFO loginInfo){ // full process of sending LOGININFO structure to server and receiving confirmation; debugging printing included
    LOGININFO loginInfo2 = loginInfo;
    // For testing purposes
    loginInfo.PrintLoginInfo(); // For testing purposes
    printf("Players found:  ");
    for(unsigned int i=0; i<loginInfo.playersFound.size(); i++){
        printf("%d ", loginInfo.playersFound[i]);
    }
    printf("\n");

    /* Sending login info to server (and receiving confirmation back) */
    loginInfo2 = Talk2ServerLogin(loginInfo);

    #ifdef DEBUG
        loginInfo2.PrintLoginInfo();
        printf("Players found:  ");
        for(unsigned int i=0; i<loginInfo2.playersFound.size(); i++){
            printf("%d ", loginInfo2.playersFound[i]);
        }
        printf("\n");

        printf("Data has been sent to the server!\n");
    #endif

    return loginInfo2;

}

GAMESTATE Talk2ServerGameState(GAMESTATE gameState){
    int n;

    /* Creating buffer, sending it to server */
    BUF RecvBuf(2000); /* message buffer for receiving a message -- think the max for login and game is well below 2000 bytes, but using this just in case */
    BUF SendBuf; /* message buffer for sending a response */

    SendBuf = createBuffer(gameState);
    printf("Dummy game state:  \n");
    officialGameState.PrintGameState();

    n = write(SocketFD, SendBuf.data(), SendBuf.size());
    if (n < 0){   
        FatalError("writing to socket failed");
    }
    
    /* Getting response from server and closing the socket */
    n = read(SocketFD, RecvBuf.data(), 2000);
    if (n < 0) {   
        FatalError("reading from socket failed");
    }
    RecvBuf.resize(n);

    // Wrapup:  Getting new structure and returning it

    gameState = parsingGameArguments(RecvBuf);
    return gameState;

}

GAMESTATE ServerGameStateRead(){
    int n;
    GAMESTATE gameState;

    /* Creating buffer, sending it to server */
    BUF RecvBuf(2000); /* message buffer for receiving a message -- think the max for login and game is well below 2000 bytes, but using this just in case */
    BUF SendBuf; /* message buffer for sending a response */
    
    /* Getting response from server and closing the socket */
    n = read(SocketFD, RecvBuf.data(), 2000);
    if (n < 0) {   
        FatalError("reading from socket failed");
    }
    RecvBuf.resize(n);

    // Wrapup:  Getting new structure and returning it

    gameState = parsingGameArguments(RecvBuf);
    return gameState;
}

void ServerGameStateWrite(GAMESTATE gameState){
    int n;

    /* Creating buffer, sending it to server */
    BUF RecvBuf(2000); /* message buffer for receiving a message -- think the max for login and game is well below 2000 bytes, but using this just in case */
    BUF SendBuf; /* message buffer for sending a response */

    SendBuf = createBuffer(gameState);
    //printf("Dummy game state:  \n");
    //officialGameState.PrintGameState();

    n = write(SocketFD, SendBuf.data(), SendBuf.size());
    if (n < 0){   
        FatalError("writing to socket failed");
    }
    

}


/******** GUI Functions **************************************************/
GtkWidget *Window;
GtkWidget *MainDisplay;
GtkWidget *Entry;
GtkWidget *InputError;

typedef struct gamewindow{
    GtkWidget *MainDisplaySub;

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
        GtkWidget *checkButton;
        GtkWidget *InputError;
        GtkWidget *helpMenuButton;
        GtkWidget *ShutdownButton;

    GtkWidget *miscBox;
        GtkWidget *pot;
        GtkWidget *callAmount;
        GtkWidget *playerNumber;
        GtkWidget *turn;


    GtkWidget *playersAndCardsBox;
    GtkWidget *movesAndLogBox;

} GameWindow;
GameWindow gameWindow;

std::string mainDeckRef[4][13] = {
    {"assets/Hearts/Heart 2.png", "assets/Hearts/Heart 3.png", "assets/Hearts/Heart 4.png", "assets/Hearts/Heart 5.png", 
        "assets/Hearts/Heart 6.png", "assets/Hearts/Heart 7.png", "assets/Hearts/Heart 8.png", "assets/Hearts/Heart 9.png", "assets/Hearts/Heart 10.png", 
        "assets/Hearts/Heart J.png", "assets/Hearts/Heart Q.png", "assets/Hearts/Heart K.png", "assets/Hearts/Heart A.png"},
    {"assets/Clovers/Clover 2.png", "assets/Clovers/Clover 3.png", "assets/Clovers/Clover 4.png", "assets/Clovers/Clover 5.png", 
        "assets/Clovers/Clover 6.png", "assets/Clovers/Clover 7.png", "assets/Clovers/Clover 8.png", "assets/Clovers/Clover 9.png", "assets/Clovers/Clover 10.png", 
        "assets/Clovers/Clover J.png", "assets/Clovers/Q Clover.png", "assets/Clovers/Clover K.png", "assets/Clovers/Clover A.png"},
    {"assets/Diamonds/Diamond 2.png", "assets/Diamonds/Diamond 3.png", "assets/Diamonds/Diamond 4.png", "assets/Diamonds/Diamond 5.png", 
        "assets/Diamonds/Diamond 6.png", "assets/Diamonds/Diamond 7.png", "assets/Diamonds/Diamond 8.png", "assets/Diamonds/Diamond 9.png", "assets/Diamonds/Diamond 10.png", 
        "assets/Diamonds/Diamond J.png", "assets/Diamonds/Diamond Q.png", "assets/Diamonds/Diamond K.png", "assets/Diamonds/Diamond A.png"},
    {"assets/Spades/Spades 2.png", "assets/Spades/Spades 3.png", "assets/Spades/Spades 4.png", "assets/Spades/Spades 5.png", 
        "assets/Spades/Spades 6.png", "assets/Spades/Spades 7.png", "assets/Spades/Spades 8.png", "assets/Spades/Spades 9.png", "assets/Spades/Spades 10.png", 
        "assets/Spades/Spades J.png", "assets/Spades/Spades Q.png", "assets/Spades/Spades K.png", "assets/Spades/Spades A.png"}
};

std::string anteaterRef = "assets/Anteater.png";
std::string backRef = "assets/Back.png";

#define CARD_W 100
#define CARD_H 150

// Display function declarations
void NameWindow(LOGININFO &loginInfo);
void PasswordWindow(LOGININFO &loginInfo);
void NumPlayersWindow(LOGININFO &loginInfo);
void PlayerNumWindow(LOGININFO &loginInfo);
void PlayerTypeWindow(LOGININFO &loginInfo);
void WaitScreenWindow(LOGININFO &loginInfo);

gboolean WaitFreeze(gpointer na);

void InitGameScreenWindow(GAMESTATE &gameState, int playerNum);
void UpdateGameScreenWindow(GAMESTATE &gameState, int playerNum, int definitelyNotTurn);

// Helper functions
    void resetMainDisplay(){
        GList *children = gtk_container_get_children(GTK_CONTAINER(MainDisplay)); // returns a doubly-linked list of pointers to children GtkWidgets
        GList *i;

        for(i = children; i != NULL; i = g_list_next(i)){ // looping through the list
            gtk_container_remove(GTK_CONTAINER(MainDisplay), GTK_WIDGET(i->data));
        }

        // Wrapup:  Freeing list pointer
        g_list_free(children);
    }

    GtkWidget *createScaledImage(const char *filename, int w, int h){
        GError *error = NULL;
        GtkWidget *newImage;

        GdkPixbuf *pixBuf = gdk_pixbuf_new_from_file_at_scale(filename, w, h, TRUE, &error);

        char msg[500];
        sprintf(msg, "Couldn't find %s", filename);

        if(error != NULL){
            g_error_free(error);
            newImage = gtk_label_new(msg);
            return newImage;
        }

        newImage = gtk_image_new_from_pixbuf(pixBuf);
        g_object_unref(pixBuf);

        // Wrapup
        return newImage;
    }

    void setScaledImage(GtkWidget *element, const char *filename, int w, int h){
        GError *error = NULL;
        GtkWidget *newImage;

        GdkPixbuf *pixBuf = gdk_pixbuf_new_from_file_at_scale(filename, w, h, TRUE, &error);

        char msg[500];
        sprintf(msg, "Couldn't find %s", filename);

        if(error != NULL){
            g_error_free(error);
            gtk_label_set_text(GTK_LABEL(element), msg);
            //newImage = gtk_label_new(msg);
            return;
        }

        gtk_image_set_from_pixbuf(GTK_IMAGE(element), pixBuf);
        newImage = gtk_image_new_from_pixbuf(pixBuf);
        g_object_unref(pixBuf);

        // Wrapup
        return;
    }


// Getting client name
    void Name_SubmitButton_onClick(GtkWidget *button, gpointer loginData){
        LOGININFO *loginInfo = (LOGININFO *)loginData; // have to typecast the gpointer

        const char *name;
        name = gtk_entry_get_text(GTK_ENTRY(Entry)); // returns const char

        // IMPORTANT!
        strcpy(loginInfo->playerName, name);

        // Moving on to next page
        resetMainDisplay();
        PasswordWindow(*loginInfo);
    }
    void NameWindow(LOGININFO &loginInfo){
        GtkWidget *TextPrompt;
        GtkWidget *SubmitButton;


        /* on top, prompt; then, going down, Text Prompt, Entry, and Submit Button */
        TextPrompt = gtk_label_new("Welcome to Anteater Poker!\nPlease enter your name (40 characters max, no special characters)");
        Entry = gtk_entry_new();
        SubmitButton = gtk_button_new_with_label("Confirm");

        gtk_container_add(GTK_CONTAINER(MainDisplay), TextPrompt);
        gtk_container_add(GTK_CONTAINER(MainDisplay), Entry);
        gtk_container_add(GTK_CONTAINER(MainDisplay), SubmitButton);

        g_signal_connect(SubmitButton, "clicked", G_CALLBACK(Name_SubmitButton_onClick), &loginInfo); // for the rest of the main menu code, loginInfo is treated as a pointer

        gtk_widget_show_all(Window);
    }

// Getting password
    void Password_SubmitButton_onClick(GtkWidget *button, gpointer loginData){
        LOGININFO *loginInfo = (LOGININFO *)loginData;

        const char *password;
        password = gtk_entry_get_text(GTK_ENTRY(Entry));

        
        if(loginInfo->isHost){
            // IMPORTANT!
            strcpy(loginInfo->password, password);
        } else{
            if(!loginInfo->isHost && strcmp(password, loginInfo->password) != 0){
                gtk_label_set_text(GTK_LABEL(InputError), "Not a recognized room password.  Please try again.");
                return;
            }
        }
        

        // Moving on to next page if player is host, or non-host player inputs correct password
        resetMainDisplay();
        if(loginInfo->isHost){
            NumPlayersWindow(*loginInfo);
        } else{ // non-host player doesn't need to access this page
            PlayerNumWindow(*loginInfo);
        }
        
    }
    void PasswordWindow(LOGININFO &loginInfo){
        GtkWidget *TextPrompt;
        // GtkWidget *Entry; now a global variable (not got practice -- FIX THIS)
        GtkWidget *SubmitButton;

        /* on top, prompt; then, going down, Text Prompt, Entry, and Submit Button */
        if(loginInfo.isHost){
            TextPrompt = gtk_label_new("Please create a password for the room (40 characters max, no special characters):  ");
        } else{
            TextPrompt = gtk_label_new("Please enter the password for the room:  ");
        }
        
        Entry = gtk_entry_new();
        SubmitButton = gtk_button_new_with_label("Confirm");
        InputError = gtk_label_new("");

        gtk_container_add(GTK_CONTAINER(MainDisplay), TextPrompt);
        gtk_container_add(GTK_CONTAINER(MainDisplay), Entry);
        gtk_container_add(GTK_CONTAINER(MainDisplay), SubmitButton);
        gtk_container_add(GTK_CONTAINER(MainDisplay), InputError);

        g_signal_connect(SubmitButton, "clicked", G_CALLBACK(Password_SubmitButton_onClick), &loginInfo); // for the rest of the main menu code, loginInfo is treated as a pointer

        gtk_widget_show_all(Window);

    }

// Getting the number of players (if applicable)
    // Think I could just create a struct w/ loginInfo and the number, but I'd rather just hard code it all right now
    void NumPlayers_afterClick(LOGININFO &loginInfo, int num){
        // IMPORTANT!
        loginInfo.numPlayers = num;
        loginInfo.playersFound.resize(num); // IMPORTANT!

        // Wrapup
        resetMainDisplay();
        PlayerNumWindow(loginInfo);

    }
    void NumPlayers_Two_onClick(GtkWidget *button, gpointer loginData){
        LOGININFO *loginInfo = (LOGININFO *)loginData;
        NumPlayers_afterClick(*loginInfo, 2);
    }
    void NumPlayers_Three_onClick(GtkWidget *button, gpointer loginData){
        LOGININFO *loginInfo = (LOGININFO *)loginData;
        NumPlayers_afterClick(*loginInfo, 3);
    }
    void NumPlayers_Four_onClick(GtkWidget *button, gpointer loginData){
        LOGININFO *loginInfo = (LOGININFO *)loginData;
        NumPlayers_afterClick(*loginInfo, 4);
    }
    void NumPlayers_Five_onClick(GtkWidget *button, gpointer loginData){
        LOGININFO *loginInfo = (LOGININFO *)loginData;
        NumPlayers_afterClick(*loginInfo, 5);
    }
    void NumPlayers_Six_onClick(GtkWidget *button, gpointer loginData){
        LOGININFO *loginInfo = (LOGININFO *)loginData;
        NumPlayers_afterClick(*loginInfo, 6);
    }
    void NumPlayers_Seven_onClick(GtkWidget *button, gpointer loginData){
        LOGININFO *loginInfo = (LOGININFO *)loginData;
        NumPlayers_afterClick(*loginInfo, 7);
    }
    void NumPlayers_Eight_onClick(GtkWidget *button, gpointer loginData){
        LOGININFO *loginInfo = (LOGININFO *)loginData;
        NumPlayers_afterClick(*loginInfo, 8);
    }
    void NumPlayers_Nine_onClick(GtkWidget *button, gpointer loginData){
        LOGININFO *loginInfo = (LOGININFO *)loginData;
        NumPlayers_afterClick(*loginInfo, 9);
    }
    void NumPlayers_Ten_onClick(GtkWidget *button, gpointer loginData){
        LOGININFO *loginInfo = (LOGININFO *)loginData;
        NumPlayers_afterClick(*loginInfo, 10);
    }
    void NumPlayersWindow(LOGININFO &loginInfo){
        GtkWidget *TextPrompt;
        GtkWidget *button[11]; // ignore 0 and 1, for simplicity

        if(loginInfo.numPlayers != 0){
                printf("Hmm...host's setting the number of players, but it seems like someone else already has?\n");
        }

        // Creating items
        TextPrompt = gtk_label_new("Please select the number of players expected (between 2 and 10, inclusively):  ");
        for(int i=2; i<=10; i++){
            char option[10];
            sprintf(option, " %d ", i);
            button[i] = gtk_button_new_with_label(option);
        }

        // Adding items to main display
        gtk_container_add(GTK_CONTAINER(MainDisplay), TextPrompt);
        for(int i=2; i<=10; i++){
            gtk_container_add(GTK_CONTAINER(MainDisplay), button[i]);
        }

        // Setting up button controlls
        g_signal_connect(button[2], "clicked", G_CALLBACK(NumPlayers_Two_onClick), &loginInfo);
        g_signal_connect(button[3], "clicked", G_CALLBACK(NumPlayers_Three_onClick), &loginInfo);
        g_signal_connect(button[4], "clicked", G_CALLBACK(NumPlayers_Four_onClick), &loginInfo);
        g_signal_connect(button[5], "clicked", G_CALLBACK(NumPlayers_Five_onClick), &loginInfo);
        g_signal_connect(button[6], "clicked", G_CALLBACK(NumPlayers_Six_onClick), &loginInfo);
        g_signal_connect(button[7], "clicked", G_CALLBACK(NumPlayers_Seven_onClick), &loginInfo);
        g_signal_connect(button[8], "clicked", G_CALLBACK(NumPlayers_Eight_onClick), &loginInfo);
        g_signal_connect(button[9], "clicked", G_CALLBACK(NumPlayers_Nine_onClick), &loginInfo);
        g_signal_connect(button[10], "clicked", G_CALLBACK(NumPlayers_Ten_onClick), &loginInfo);


        // Display
        gtk_widget_show_all(Window);
    }


// Getting the player number/position
    void PlayerNum_afterClick(LOGININFO &loginInfo, int num){
        if(num < 0 || (unsigned int)num >= loginInfo.playersFound.size()){
            printf("Huh...it looks like the player somehow managed to pick a player position that shouldn't have been available.  This might create misbehavior and errors in the program.\n");
            return;
        }
        
        // IMPORTANT!
        loginInfo.playerNum = num;
        loginInfo.playersFound[num] = 1;

        // Wrapup
        resetMainDisplay();
        PlayerTypeWindow(loginInfo);
    }
    void PlayerZero_onClick(GtkWidget *button, gpointer loginData){
        LOGININFO *loginInfo = (LOGININFO *)loginData;
        PlayerNum_afterClick(*loginInfo, 0);

    }
    void PlayerOne_onClick(GtkWidget *button, gpointer loginData){
        LOGININFO *loginInfo = (LOGININFO *)loginData;
        PlayerNum_afterClick(*loginInfo, 1);

    }
    void PlayerTwo_onClick(GtkWidget *button, gpointer loginData){
        LOGININFO *loginInfo = (LOGININFO *)loginData;
        PlayerNum_afterClick(*loginInfo, 2);

    }
    void PlayerThree_onClick(GtkWidget *button, gpointer loginData){
        LOGININFO *loginInfo = (LOGININFO *)loginData;
        PlayerNum_afterClick(*loginInfo, 3);

    }
    void PlayerFour_onClick(GtkWidget *button, gpointer loginData){
        LOGININFO *loginInfo = (LOGININFO *)loginData;
        PlayerNum_afterClick(*loginInfo, 4);

    }
    void PlayerFive_onClick(GtkWidget *button, gpointer loginData){
        LOGININFO *loginInfo = (LOGININFO *)loginData;
        PlayerNum_afterClick(*loginInfo, 5);

    }
    void PlayerSix_onClick(GtkWidget *button, gpointer loginData){
        LOGININFO *loginInfo = (LOGININFO *)loginData;
        PlayerNum_afterClick(*loginInfo, 6);

    }
    void PlayerSeven_onClick(GtkWidget *button, gpointer loginData){
        LOGININFO *loginInfo = (LOGININFO *)loginData;
        PlayerNum_afterClick(*loginInfo, 7);

    }
    void PlayerEight_onClick(GtkWidget *button, gpointer loginData){
        LOGININFO *loginInfo = (LOGININFO *)loginData;
        PlayerNum_afterClick(*loginInfo, 8);

    }
    void PlayerNine_onClick(GtkWidget *button, gpointer loginData){
        LOGININFO *loginInfo = (LOGININFO *)loginData;
        PlayerNum_afterClick(*loginInfo, 9);

    }
    void PlayerNumWindow(LOGININFO &loginInfo){
        GtkWidget *TextPrompt;
        GtkWidget *button[10];

        // Creating items
        char prompt[300];
        sprintf(prompt, "There are %d players.\n  Please select an available number/position:  ", loginInfo.numPlayers);
        TextPrompt = gtk_label_new(prompt);
        for(int i=0; i<10; i++){
            char option[20];
            sprintf(option, "Player %d", i+1);
            button[i] = gtk_button_new_with_label(option);
        }

        // Adding items to main display
        gtk_container_add(GTK_CONTAINER(MainDisplay), TextPrompt);
        // NOTE:  Only including/displaying valid options!
        for(unsigned int i=0; i<10; i++){
            if(i < loginInfo.playersFound.size() && loginInfo.playersFound[i] == 0){
                gtk_container_add(GTK_CONTAINER(MainDisplay), button[i]);
            }
        }

        // Setting up button controls
        g_signal_connect(button[0], "clicked", G_CALLBACK(PlayerZero_onClick), &loginInfo);
        g_signal_connect(button[1], "clicked", G_CALLBACK(PlayerOne_onClick), &loginInfo);
        g_signal_connect(button[2], "clicked", G_CALLBACK(PlayerTwo_onClick), &loginInfo);
        g_signal_connect(button[3], "clicked", G_CALLBACK(PlayerThree_onClick), &loginInfo);
        g_signal_connect(button[4], "clicked", G_CALLBACK(PlayerFour_onClick), &loginInfo);
        g_signal_connect(button[5], "clicked", G_CALLBACK(PlayerFive_onClick), &loginInfo);
        g_signal_connect(button[6], "clicked", G_CALLBACK(PlayerSix_onClick), &loginInfo);
        g_signal_connect(button[7], "clicked", G_CALLBACK(PlayerSeven_onClick), &loginInfo);
        g_signal_connect(button[8], "clicked", G_CALLBACK(PlayerEight_onClick), &loginInfo);
        g_signal_connect(button[9], "clicked", G_CALLBACK(PlayerNine_onClick), &loginInfo);


        // Display
        gtk_widget_show_all(Window);
    }

// Getting player type
    void PlayerType_afterClick(LOGININFO &loginInfo, int num){
        // IMPORTANT!
        loginInfo.playerType = num;

        // Super important! -- Sending data to server, receiving confirmation back of what the server received
        SendServerLogin(loginInfo); // sends info to server; does ignore a few defaults in the server, so keeping the original sent by the user instead (hence the printf statements in SendServerLogin to make sure)

        // Wrapup
        resetMainDisplay();
        WaitScreenWindow(loginInfo);
    }
    void HumanButton_onClick(GtkWidget *button, gpointer loginData){
        LOGININFO *loginInfo = (LOGININFO *)loginData;
        PlayerType_afterClick(*loginInfo, Human);
    }
    void ComputerButton_onClick(GtkWidget *button, gpointer loginData){
        LOGININFO *loginInfo = (LOGININFO *)loginData;
        PlayerType_afterClick(*loginInfo, Computer);
    }
    void PlayerTypeWindow(LOGININFO &loginInfo){
        GtkWidget *TextPrompt;
        GtkWidget *HumanButton;
        GtkWidget *ComputerButton;

        // Creating display elements
        TextPrompt = gtk_label_new("Do you want this player to be a human-controlled player or a computer-controlled player?");
        HumanButton = gtk_button_new_with_label("Human");
        ComputerButton = gtk_button_new_with_label("Computer");

        // Adding/staging elements
        gtk_container_add(GTK_CONTAINER(MainDisplay), TextPrompt);
        gtk_container_add(GTK_CONTAINER(MainDisplay), HumanButton);
        gtk_container_add(GTK_CONTAINER(MainDisplay), ComputerButton);

        // Setting up button controls
        g_signal_connect(HumanButton, "clicked", G_CALLBACK(HumanButton_onClick), &loginInfo);
        g_signal_connect(ComputerButton, "clicked", G_CALLBACK(ComputerButton_onClick), &loginInfo);

        // Display
        gtk_widget_show_all(Window);
    }

// Wait screen (also the exit screen for the alpha version)
    void WaitScreenWindow(LOGININFO &loginInfo){
        printf("Got to the wait screen!\n");
        playerLoginInfo = loginInfo;
        GtkWidget *TextMessage;
        GtkWidget *AlphaEndMessage;

        TextMessage = gtk_label_new("Waiting for other players to join...");
        AlphaEndMessage = gtk_label_new("Thank you for participating!  The server GUI should now display some info about your login\n\nHeads up:  This screen may freeze after 3 seconds until the last person logs on.  \nDon't worry, though--you should still be able to do other things on your computer while in this wait menu!\n  If you have to leave the game though, use Control + C and notify the other players, because they will have to restart the server/login process.");

        // printf("Labels created.\n");

        // Adding/staging elements
        gtk_container_add(GTK_CONTAINER(MainDisplay), TextMessage);
        gtk_container_add(GTK_CONTAINER(MainDisplay), AlphaEndMessage);

        // printf("Staging complete.\n");

        // Displaying elements
        gtk_widget_show_all(Window);

        // printf("Elements displayed.\n");


        // Getting initial gamestate
        // printf("Going to stalemate.\n");
        

        // Want to force a decently sized delay before going to the "freeze" state
        // ZZZ:  FIX THIS!
            g_timeout_add(3000, WaitFreeze, NULL); // calls function after 3 seconds


        //////
        return;
    }
    gboolean WaitFreeze(gpointer na){ // using this to get the freeze to happen during the wait menu window
        /*if(officialGameState.playerTurn == playerLoginInfo.playerNum){
            ServerGameStateWrite(officialGameState);
        }

        officialGameState = ServerGameStateRead(officialGameState);*/
        officialGameState = Talk2ServerGameState(officialGameState);

        // Go to initial game window
        resetMainDisplay();
        InitGameScreenWindow(officialGameState, playerLoginInfo.playerNum);

        return FALSE; // run timeout only once
    }

// Initial game window
    gboolean WaitTurnFreeze(gpointer na){ // using this to get the freeze to happen during the wait menu window
        officialGameState = ServerGameStateRead();

        UpdateGameScreenWindow(officialGameState, playerLoginInfo.playerNum, 0);

        // Wrapup:  Determine whether to run the loop again
        if(officialGameState.playerTurn == playerLoginInfo.playerNum){
            return FALSE; // stop running
        }
        return TRUE; // keep running (still not your turn)
    }

    void defaultButtonClick(GtkWidget *button, gpointer clickData){
        printf("A button was clicked!\n");
    }
    void callClick(GtkWidget *button, gpointer clickData){
        int proposedCall = officialGameState.callAmount - officialGameState.players[playerLoginInfo.playerNum].bet;
        printf("Proposed call amount:  %d\nPlayer's current score:  %d\n", proposedCall, officialGameState.players[playerLoginInfo.playerNum].score);
        if(proposedCall >= officialGameState.players[playerLoginInfo.playerNum].score || proposedCall == 0){
            gtk_label_set_text(GTK_LABEL(gameWindow.InputError), "Illegal move:  call.  \nCall must be used to match the current bet and cannot be used for all in.  \nPlease try again.");
            return;
        }

        // Valid call
        officialGameState.players[playerLoginInfo.playerNum].bet += proposedCall;
        officialGameState.players[playerLoginInfo.playerNum].score -= proposedCall;
        officialGameState.pot += proposedCall;

        // Wrapup
        gtk_label_set_text(GTK_LABEL(gameWindow.InputError), "");
        UpdateGameScreenWindow(officialGameState, playerLoginInfo.playerNum, 1);
        ServerGameStateWrite(officialGameState);
        g_timeout_add(500, WaitTurnFreeze, NULL);
    }
    void raiseClick(GtkWidget *button, gpointer clickData){
        const char *raise; 
        raise = gtk_entry_get_text(GTK_ENTRY(gameWindow.raiseEntry));
        int raiseAmount = atoi(raise);
        int proposedRaise = officialGameState.callAmount + raiseAmount - officialGameState.players[playerLoginInfo.playerNum].bet; // full bet minus what has already been bet

        if(raiseAmount <= 0 || proposedRaise >= officialGameState.players[playerLoginInfo.playerNum].score){
            gtk_label_set_text(GTK_LABEL(gameWindow.InputError), "Illegal move:  raise.  \nRaise must be used to match the current bet + some extra (extra specified by user), \nand it cannot be used for all in.  Please try again.");
            return;
        }

        // Valid raise
        officialGameState.greatest = officialGameState.playerTurn; // new greatest better
        officialGameState.players[playerLoginInfo.playerNum].bet += proposedRaise;
        officialGameState.players[playerLoginInfo.playerNum].score -= proposedRaise;
        officialGameState.pot += proposedRaise;
        officialGameState.callAmount += raiseAmount;

        // Wrapup
        gtk_label_set_text(GTK_LABEL(gameWindow.InputError), "");
        UpdateGameScreenWindow(officialGameState, playerLoginInfo.playerNum, 1);
        ServerGameStateWrite(officialGameState);
        g_timeout_add(750, WaitTurnFreeze, NULL);
    }
    void foldClick(GtkWidget *button, gpointer clickData){
        // Valid fold
        officialGameState.players[playerLoginInfo.playerNum].isInHand = 0;

        // Wrapup
        gtk_label_set_text(GTK_LABEL(gameWindow.InputError), "");
        UpdateGameScreenWindow(officialGameState, playerLoginInfo.playerNum, 1);
        ServerGameStateWrite(officialGameState);
        g_timeout_add(750, WaitTurnFreeze, NULL);
    }
    void checkClick(GtkWidget *button, gpointer clickData){
        // Player can check if and only if the player has in total bet the same amount as the call amount
        if(officialGameState.players[playerLoginInfo.playerNum].bet != officialGameState.callAmount){
            gtk_label_set_text(GTK_LABEL(gameWindow.InputError), "Illegal move:  check.  \nPlayers can only check when their total bet matches the current call amount.  \nPlease try again.");
            return;
        }

        // Valid check

        // Wrapup
        gtk_label_set_text(GTK_LABEL(gameWindow.InputError), "");
        UpdateGameScreenWindow(officialGameState, playerLoginInfo.playerNum, 1);
        ServerGameStateWrite(officialGameState);
        g_timeout_add(750, WaitTurnFreeze, NULL);
    }

    void InitGameScreenWindow(GAMESTATE &gameState, int playerNum){
        /*   Creating entries   */
        // Player entries
            char description[500];    
            for(unsigned int i=0; i<gameState.players.size(); i++){
                PLAYER player = gameState.players[i];
                if(i == playerLoginInfo.playerNum){
                    officialPlayerSocket = player.playerSocket; // ID'ing information used at the beginning of each round to determine if player number has changed
                }
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
                if(foundCard.getValue() == Anteater){
                    gameWindow.playerCards[i] = createScaledImage(anteaterRef.c_str(), CARD_W, CARD_H);
                    continue;
                }
                cardStr = mainDeckRef[foundCard.getSuit()][foundCard.getValue()];
                gameWindow.playerCards[i] = createScaledImage(cardStr.c_str(), CARD_W, CARD_H);
            }

            if(gameState.allCards.size() < 3){
                printf("ERROR:  When trying to display community cards, noticed that there are only two piles.  This is not enough.  Exiting function...\n");
                return;
            }
            for(int i=0; i<5; i++){
                int r = gameState.round;
                if(  r == Preflop || (r == Flop && i>2) || (r == Turn && i>3)  ){
                    gameWindow.commCards[i] = createScaledImage(backRef.c_str(), CARD_W, CARD_H);
                } else{
                    foundCard = gameState.allCards[gameState.allCards.size()-2][i]; // ex:  player1, player2, player3, _commCards_, restOfCards
                    if(foundCard.getValue() == Anteater){
                        gameWindow.commCards[i] = createScaledImage(anteaterRef.c_str(), CARD_W, CARD_H);
                        continue;
                    } 
                    cardStr = mainDeckRef[foundCard.getSuit()][foundCard.getValue()];
                    gameWindow.commCards[i] = createScaledImage(cardStr.c_str(), CARD_W, CARD_H);
                }
            }
        
        // Miscellaneous entries
            char miscMsg[500];
            sprintf(miscMsg, "Pot:  %d points", gameState.pot);
            gameWindow.pot = gtk_label_new(miscMsg);
            sprintf(miscMsg, "Total Call amount:  %d points", gameState.callAmount);
            gameWindow.callAmount = gtk_label_new(miscMsg);
            sprintf(miscMsg, "You are:  Player %d", playerLoginInfo.playerNum + 1);
            gameWindow.playerNumber = gtk_label_new(miscMsg);
            sprintf(miscMsg, "It is Player %d's turn", gameState.playerTurn + 1);
            gameWindow.turn = gtk_label_new(miscMsg);

        
        // Player input entries
            gameWindow.callButton = gtk_button_new_with_label("Call");
            gameWindow.raiseButton = gtk_button_new_with_label("Confirm Raise");
            gameWindow.raiseEntry = gtk_entry_new();
            gameWindow.foldButton = gtk_button_new_with_label("Fold");
            gameWindow.checkButton = gtk_button_new_with_label("Check");
            gameWindow.InputError = gtk_label_new("");
            gameWindow.helpMenuButton = gtk_button_new_with_label("Help Menu");
            gameWindow.ShutdownButton = gtk_button_new_with_label("End game and stop server");

        // Logging
            gameWindow.logEvents = gtk_text_view_new();
            gameWindow.logEventsBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(gameWindow.logEvents));
            gtk_text_view_set_editable(GTK_TEXT_VIEW(gameWindow.logEvents), FALSE);
            gameWindow.logScrollBox = gtk_scrolled_window_new(NULL, NULL);

        
        /*  Creating extra boxes; Staging  */ 
        // Boxes
            // Main Display -- now done in main function instead
            gameWindow.MainDisplaySub = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
            gtk_box_pack_start(GTK_BOX(MainDisplay), gameWindow.MainDisplaySub, TRUE, TRUE, 5);
            

            // Overarching boxes
            gameWindow.playersAndCardsBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
            gtk_box_pack_start(GTK_BOX(gameWindow.MainDisplaySub), gameWindow.playersAndCardsBox, TRUE, TRUE, 0);
            gameWindow.movesAndLogBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
            gtk_box_pack_start(GTK_BOX(gameWindow.MainDisplaySub), gameWindow.movesAndLogBox, TRUE, TRUE, 0);

            // Players
            gameWindow.playerIconsBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
            gtk_box_pack_start(GTK_BOX(gameWindow.playersAndCardsBox), gameWindow.playerIconsBox, FALSE, FALSE, 2);
            for(unsigned int i=0; i<gameState.players.size(); i++){
                gtk_box_pack_start(GTK_BOX(gameWindow.playerIconsBox), gameWindow.playerIcons[i], FALSE, TRUE, 0);
            }

            // CardBox
            gameWindow.cardsBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
            gtk_box_pack_start(GTK_BOX(gameWindow.playersAndCardsBox), gameWindow.cardsBox, FALSE, TRUE, 2);
            // CommCards
            gameWindow.commCardsBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); // reducing space btw cards
            gtk_box_pack_start(GTK_BOX(gameWindow.cardsBox), gameWindow.commCardsBox, FALSE, TRUE, 2);
            for(int i=0; i<5; i++){
                gtk_box_pack_start(GTK_BOX(gameWindow.commCardsBox), gameWindow.commCards[i], FALSE, TRUE, 0);
            }
            // PlayerCards
            gameWindow.playerCardsBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); // reducing space btw cards
            gtk_box_pack_start(GTK_BOX(gameWindow.cardsBox), gameWindow.playerCardsBox, FALSE, TRUE, 2);
            for(int i=0; i<2; i++){
                gtk_box_pack_start(GTK_BOX(gameWindow.playerCardsBox), gameWindow.playerCards[i], FALSE, TRUE, 0);
            }

            // Miscellaneous
                gameWindow.miscBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
                gtk_box_pack_start(GTK_BOX(gameWindow.playersAndCardsBox), gameWindow.miscBox, FALSE, TRUE, 2);
                gtk_box_pack_start(GTK_BOX(gameWindow.miscBox), gameWindow.pot, FALSE, TRUE, 0);
                gtk_box_pack_start(GTK_BOX(gameWindow.miscBox), gameWindow.callAmount, FALSE, TRUE, 0);
                gtk_box_pack_start(GTK_BOX(gameWindow.miscBox), gameWindow.playerNumber, FALSE, TRUE, 0);
                gtk_box_pack_start(GTK_BOX(gameWindow.miscBox), gameWindow.turn, FALSE, TRUE, 0);

            


            // Moves
            if(playerLoginInfo.playerType != Computer){
                gameWindow.playerInputBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
                gtk_box_pack_start(GTK_BOX(gameWindow.movesAndLogBox), gameWindow.playerInputBox, FALSE, TRUE, 2);
                
                gtk_box_pack_start(GTK_BOX(gameWindow.playerInputBox), gameWindow.callButton, FALSE, FALSE, 2);
                gameWindow.raiseBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
                gtk_box_pack_start(GTK_BOX(gameWindow.playerInputBox), gameWindow.raiseBox, FALSE, FALSE, 2);
                gtk_box_pack_start(GTK_BOX(gameWindow.raiseBox), gameWindow.raiseEntry, FALSE, FALSE, 2);
                gtk_box_pack_start(GTK_BOX(gameWindow.raiseBox), gameWindow.raiseButton, FALSE, FALSE, 2);
                gtk_box_pack_start(GTK_BOX(gameWindow.playerInputBox), gameWindow.foldButton, FALSE, FALSE, 2);
                gtk_box_pack_start(GTK_BOX(gameWindow.playerInputBox), gameWindow.checkButton, FALSE, FALSE, 2);
                gtk_box_pack_start(GTK_BOX(gameWindow.playerInputBox), gameWindow.InputError, FALSE, FALSE, 2);
                // gtk_container_add(GTK_CONTAINER(gameWindow.playerInputBox), gameWindow.InputError); -- Don't want to display this unless there is an actual input error
                gtk_box_pack_start(GTK_BOX(gameWindow.playerInputBox), gameWindow.helpMenuButton, FALSE, FALSE, 2);
                gtk_box_pack_start(GTK_BOX(gameWindow.playerInputBox), gameWindow.ShutdownButton, FALSE, FALSE, 2);
            }

            
        
        /*   Activating buttons   */
            g_signal_connect(gameWindow.callButton, "clicked", G_CALLBACK(callClick), NULL);
            g_signal_connect(gameWindow.raiseButton, "clicked", G_CALLBACK(raiseClick), NULL);
            g_signal_connect(gameWindow.foldButton, "clicked", G_CALLBACK(foldClick), NULL);
            g_signal_connect(gameWindow.checkButton, "clicked", G_CALLBACK(checkClick), NULL);
            g_signal_connect(gameWindow.helpMenuButton, "clicked", G_CALLBACK(defaultButtonClick), NULL);
            g_signal_connect(gameWindow.ShutdownButton, "clicked", G_CALLBACK(defaultButtonClick), NULL);


        // Wrapup
        printf("Player cards:  ");
        printCard(officialGameState.allCards[playerLoginInfo.playerNum][0]);
        printCard(officialGameState.allCards[playerLoginInfo.playerNum][1]);
        printf("\n");
        gtk_widget_show_all(Window);

        // Hiding items if they are not supposed to be visible yet (i.e. not player's turn at start)
            if(gameState.playerTurn != playerLoginInfo.playerNum){
                gtk_widget_hide(gameWindow.callButton);
                gtk_widget_hide(gameWindow.raiseButton);
                gtk_widget_hide(gameWindow.raiseEntry);
                gtk_widget_hide(gameWindow.foldButton);
                gtk_widget_hide(gameWindow.checkButton);
                gtk_widget_hide(gameWindow.ShutdownButton);
            }

        if(officialGameState.playerTurn != playerLoginInfo.playerNum){ // not player's turn--start waiting
            g_timeout_add(500, WaitTurnFreeze, NULL);
        }
        
    }

    void UpdateGameScreenWindow(GAMESTATE &gameState, int playerNum, int definitelyNotTurn){
        // Updating player number based on possible eliminations
        if(officialGameState.round == Preflop){
            int newPlayerNumber = -1;
            for(unsigned int i=0; i<officialGameState.players.size(); i++){
                if(officialGameState.players[i].playerSocket == officialPlayerSocket){
                    playerLoginInfo.playerNum = i; // updating official record of player number
                    newPlayerNumber = i;
                    break;
                }
            }
            if(newPlayerNumber == -1){ // player not found -- has been eliminated
                printf("PLAYER HAS BEEN ELIMINATED FROM THE GAME.\n");
                close(SocketFD);
                exit(0);

                return;
            }
        }


        // Player entries
            char description[500];    
            for(unsigned int i=0; i<gameState.players.size(); i++){
                PLAYER player = gameState.players[i];
                // asdf 
                sprintf(description, "Player %d: %s\nPoints: %d\nBet: %d\nIn round: %s\nEliminated?: %s\n%s\n", i+1, player.name, player.score, player.bet, 
                    (player.isInHand) ? "yes" : "no", (player.isEliminated) ? "yes" : "no", (i == gameState.dealerPlayer) ? "DEALER" : "");
                gtk_label_set_text(GTK_LABEL(gameWindow.playerIcons[i]), description);
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
                if(foundCard.getValue() == Anteater){
                    // gameWindow.playerCards[i] = createScaledImage(anteaterRef.c_str(), CARD_W, CARD_H);
                    //gtk_image_set_from_file(GTK_IMAGE(gameWindow.playerCards[i]), anteaterRef.c_str());
                    setScaledImage(gameWindow.playerCards[i], anteaterRef.c_str(), CARD_W, CARD_H);
                    continue;
                }
                cardStr = mainDeckRef[foundCard.getSuit()][foundCard.getValue()];
                // gameWindow.playerCards[i] = createScaledImage(cardStr.c_str(), CARD_W, CARD_H);
                // gtk_image_set_from_file(GTK_IMAGE(gameWindow.playerCards[i]), cardStr.c_str());
                setScaledImage(gameWindow.playerCards[i], cardStr.c_str(), CARD_W, CARD_H);
            }

            if(gameState.allCards.size() < 3){
                printf("ERROR:  When trying to display community cards, noticed that there are only two piles.  This is not enough.  Exiting function...\n");
                return;
            }
            for(int i=0; i<5; i++){
                int r = gameState.round;
                if(  r == Preflop || (r == Flop && i>2) || (r == Turn && i>3)  ){
                    // gameWindow.commCards[i] = createScaledImage(backRef.c_str(), CARD_W, CARD_H);
                    // gtk_image_set_from_file(GTK_IMAGE(gameWindow.commCards[i]), backRef.c_str());
                    setScaledImage(gameWindow.commCards[i], backRef.c_str(), CARD_W, CARD_H);
                } else{
                    foundCard = gameState.allCards[gameState.allCards.size()-2][i]; // ex:  player1, player2, player3, _commCards_, restOfCards
                    if(foundCard.getValue() == Anteater){
                        // gameWindow.commCards[i] = createScaledImage(anteaterRef.c_str(), CARD_W, CARD_H);
                        // gtk_image_set_from_file(GTK_IMAGE(gameWindow.commCards[i]), anteaterRef.c_str());
                        setScaledImage(gameWindow.commCards[i], anteaterRef.c_str(), CARD_W, CARD_H);
                        continue;
                    } 
                    cardStr = mainDeckRef[foundCard.getSuit()][foundCard.getValue()];
                    // gameWindow.commCards[i] = createScaledImage(cardStr.c_str(), CARD_W, CARD_H);
                    // gtk_image_set_from_file(GTK_IMAGE(gameWindow.commCards[i]), cardStr.c_str());
                    setScaledImage(gameWindow.commCards[i], cardStr.c_str(), CARD_W, CARD_H);
                }
            }
        
        // Miscellaneous entries
            char miscMsg[500];
            sprintf(miscMsg, "Pot:  %d points", gameState.pot);
            gtk_label_set_text(GTK_LABEL(gameWindow.pot), miscMsg);
            sprintf(miscMsg, "Total Call amount:  %d points", gameState.callAmount);
            gtk_label_set_text(GTK_LABEL(gameWindow.callAmount), miscMsg);
            sprintf(miscMsg, "You are:  Player %d", playerLoginInfo.playerNum + 1);
            gtk_label_set_text(GTK_LABEL(gameWindow.playerNumber), miscMsg);
            sprintf(miscMsg, "It is Player %d's turn", gameState.playerTurn + 1);
            gtk_label_set_text(GTK_LABEL(gameWindow.turn), miscMsg);

        
        // Player input entries (controlling display)
        // Do NOT show the display if it is definitely not the player's turn, if the player is a computer player, if it is not the player's turn, or if the player is not in the hand
        if(definitelyNotTurn || playerLoginInfo.playerType == Computer || officialGameState.playerTurn != playerLoginInfo.playerNum || officialGameState.players[playerLoginInfo.playerNum].isInHand == 0){
            gtk_widget_hide(gameWindow.callButton);
            gtk_widget_hide(gameWindow.raiseButton);
            gtk_widget_hide(gameWindow.raiseEntry);
            gtk_widget_hide(gameWindow.foldButton);
            gtk_widget_hide(gameWindow.checkButton);
            gtk_widget_hide(gameWindow.ShutdownButton);
        } else{
            gtk_widget_show(gameWindow.callButton);
            gtk_widget_show(gameWindow.raiseButton);
            gtk_widget_show(gameWindow.raiseEntry);
            gtk_widget_show(gameWindow.foldButton);
            gtk_widget_show(gameWindow.checkButton);
            gtk_widget_show(gameWindow.ShutdownButton);
        }

        // Wrapup
        // g_timeout_add(500, WaitTurnFreeze, NULL);

    }
    




void MainMenu(int *argc, char **argv[], LOGININFO &loginInfo){
    /* initialize the GTK libraries */
    gtk_init(argc, argv);

    /* create the main, top level window */
    Window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(Window), "Poker Client View");
    gtk_window_set_default_size(GTK_WINDOW(Window), 260, 280);
    gtk_container_set_border_width (GTK_CONTAINER(Window), 10);

    g_signal_connect(Window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    /* overall vertical arrangement in the window */
    // MainDisplay = gtk_vbox_new(FALSE, 10);
    MainDisplay = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(Window), MainDisplay);

    // Running the menu
    NameWindow(loginInfo);

    gtk_widget_show_all(Window);
    gtk_main();
    
    /*
        PasswordWindow(argc, argv, loginInfo);
        NumPlayersWindow(argc, argv, loginInfo);
        PlayerNumWindow(argc, argv, loginInfo);
        PlayerTypeWindow(argc, argv, loginInfo);
    */
}


/* MAIN FUNCTION */
int main(int argc, char *argv[]){
    int PortNo;
    struct hostent *Server;

    Program = argv[0];

    if (argc < 3){  /* DEFAULT CASE:  Prompt user for server */ 
        //fprintf(stderr, "Usage: %s hostname port\n", Program);
	    //exit(10);
        char actualServer[100];
        printf("Before starting the login menu, please specify where the server actually is (e.g. an IP address; \'bondi\'; etc):  ");
        scanf(" %s", actualServer);

        Server = gethostbyname(actualServer);
        PortNo = DEFAULT_SERVERPORT;
        
    } else{ /* NON-DEFAULT CASE */
        Server = gethostbyname(argv[1]);
        PortNo = atoi(argv[2]);
    }


    if (Server == NULL)
    {   fprintf(stderr, "%s: no such host named '%s'\n", Program, argv[1]);
        exit(10);
    }

    if (PortNo <= 2000)
    {   fprintf(stderr, "%s: invalid port number %d, should be >2000\n",
		Program, PortNo);
        exit(10);
    }
    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_port = htons(PortNo);
    ServerAddress.sin_addr = *(struct in_addr*)Server->h_addr_list[0];

    // Creating socket
    // int n;
    //int SocketFD;

    /* Connecting to Server*/
    SocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (SocketFD < 0){   
        FatalError("socket creation failed");
    }
    if (connect(SocketFD, (struct sockaddr*)&ServerAddress,
            sizeof(struct sockaddr_in)) < 0){   
                FatalError("connecting to server failed");
    }


    /* LOGIN MENU */
    LOGININFO loginInfo;
    loginInfo = Talk2ServerLogin(loginInfo);
    MainMenu(&argc, &argv, loginInfo);


    // Wrapup
    close(SocketFD);
    
}


/* EOF GTK_ClockClient.c */
