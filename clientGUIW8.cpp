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


/******** GUI Functions **************************************************/
GtkWidget *Window;
GtkWidget *MainDisplay;
GtkWidget *Entry;
GtkWidget *InputError;


void NameWindow(LOGININFO &loginInfo);
void PasswordWindow(LOGININFO &loginInfo);
void NumPlayersWindow(LOGININFO &loginInfo);
void PlayerNumWindow(LOGININFO &loginInfo);
void PlayerTypeWindow(LOGININFO &loginInfo);

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

// Getting the number of players
void NumPlayers_afterClick(){

}
void NumPlayers_Two_onClick(GtkWidget *button, gpointer loginData){

}
void NumPlayers_Three_onClick(GtkWidget *button, gpointer loginData){
    
}
void NumPlayers_Four_onClick(GtkWidget *button, gpointer loginData){
    
}
void NumPlayers_Five_onClick(GtkWidget *button, gpointer loginData){
    
}
void NumPlayers_Six_onClick(GtkWidget *button, gpointer loginData){
    
}
void NumPlayers_Seven_onClick(GtkWidget *button, gpointer loginData){
    
}
void NumPlayers_Eight_onClick(GtkWidget *button, gpointer loginData){
    
}
void NumPlayers_Nine_onClick(GtkWidget *button, gpointer loginData){
    
}
void NumPlayers_Ten_onClick(GtkWidget *button, gpointer loginData){
    
}
void NumPlayersWindow(LOGININFO &loginInfo){
    return;
}

void PlayerNumWindow(LOGININFO &loginInfo){
    return;
}

// Getting the number of players (if applicable)

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

    if (argc < 3)
    {   fprintf(stderr, "Usage: %s hostname port\n", Program);
	exit(10);
    }
    Server = gethostbyname(argv[1]);
    if (Server == NULL)
    {   fprintf(stderr, "%s: no such host named '%s'\n", Program, argv[1]);
        exit(10);
    }
    PortNo = atoi(argv[2]);
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
    // For testing purposes
    loginInfo.PrintLoginInfo(); // For testing purposes
    printf("Players found:  ");
    for(unsigned int i=0; i<loginInfo.playersFound.size(); i++){
        printf("%d ", loginInfo.playersFound[i]);
    }
    printf("\n");

    /* Sending login info to server (and receiving confirmation back) */
    Talk2ServerLogin(loginInfo);

    loginInfo.PrintLoginInfo();
    printf("Players found:  ");
    for(unsigned int i=0; i<loginInfo.playersFound.size(); i++){
        printf("%d ", loginInfo.playersFound[i]);
    }
    printf("\n");

    printf("Data has been sent to the server!\n");
}


/* EOF GTK_ClockClient.c */
