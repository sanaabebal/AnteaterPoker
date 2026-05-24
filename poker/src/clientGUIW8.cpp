/* clientTextW8.cpp */

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
    int SocketFD;

    /* Connecting to Server*/
    SocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (SocketFD < 0){   
        FatalError("socket creation failed");
    }
    if (connect(SocketFD, (struct sockaddr*)&ServerAddress,
            sizeof(struct sockaddr_in)) < 0){   
                FatalError("connecting to server failed");
    }

    /* Creating buffer, sending it to server */
    BUF RecvBuf(2000); /* message buffer for receiving a message -- think the max for login and game is well below 2000 bytes, but using this just in case */
    BUF SendBuf; /* message buffer for sending a response */

    SendBuf = createBuffer(loginInfo);
    n = write(SocketFD, SendBuf.data(), 2000);
    if (n < 0){   
        FatalError("writing to socket failed");
    }
    
    /* Getting response from server and closing the socket */
    n = read(SocketFD, RecvBuf.data(), 2000);
    if (n < 0) {   
        FatalError("reading from socket failed");
    }
    close(SocketFD);

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


/******** GUI Functions **************************************************/
GtkWidget *Window;
GtkWidget *MainDisplay;
GtkWidget *Entry;
GtkWidget *InputError;

// Display function declarations
void NameWindow(LOGININFO &loginInfo);
void PasswordWindow(LOGININFO &loginInfo);
void NumPlayersWindow(LOGININFO &loginInfo);
void PlayerNumWindow(LOGININFO &loginInfo);
void PlayerTypeWindow(LOGININFO &loginInfo);
void WaitScreenWindow(LOGININFO &loginInfo);

void resetMainDisplay(){
    GList *children = gtk_container_get_children(GTK_CONTAINER(MainDisplay)); // returns a doubly-linked list of pointers to children GtkWidgets
    GList *i;

    for(i = children; i != NULL; i = g_list_next(i)){ // looping through the list
        gtk_container_remove(GTK_CONTAINER(MainDisplay), GTK_WIDGET(i->data));
    }

    // Wrapup:  Freeing list pointer
    g_list_free(children);
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
        GtkWidget *TextMessage;
        GtkWidget *AlphaEndMessage;

        TextMessage = gtk_label_new("Waiting for other players to join...");
        AlphaEndMessage = gtk_label_new("This is as far as we got for the alpha version of the anteater poker project.  Thank you for participating!  The server GUI should now display some info about your login");

        // Adding/staging elements
        gtk_container_add(GTK_CONTAINER(MainDisplay), TextMessage);
        gtk_container_add(GTK_CONTAINER(MainDisplay), AlphaEndMessage);

        // Displaying elements
        gtk_widget_show_all(Window);

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
    MainDisplay = gtk_vbox_new(FALSE, 10);
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


    /* LOGIN MENU */
    LOGININFO loginInfo;
    loginInfo = Talk2ServerLogin(loginInfo);
    MainMenu(&argc, &argv, loginInfo);
    
}


/* EOF GTK_ClockClient.c */
