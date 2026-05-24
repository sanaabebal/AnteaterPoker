All In Presents:  Anteater Poker (Version 1.0)

Authors:  Sanaa Bebal, Ava Chinn, Alonso De La Cruz Becerra, Estrella Distancia Zamora, Lauren Howe, David Lee, and Samuel Kim

Alpha version release date (Version 1.0):  May 23rd, 2026 (10 AM PDT)

Please enjoy our program!

**********************************************************
Getting Started:
* See INSTALL file for requirements to install and use this program
* See doc/Poker_UserManual.pdf for the user manual
* See doc/Poker_SoftwareSpec.pdf for the software specification

**********************************************************
Alpha Version Notes:
* The program is intended to have the user connect to the server to get the information needed for logging in, 
        allow the player to go through the login menu based on whether or not they are the host,
        and send that information from the login menu to the server, which will then be reflected on the server GUI.
        The player will then be sent to a waiting screen/exit screen, as this is as far as the program has gone at this point.
* In the event that the server is refusing to initialize because it says the port is already in use, please use the following commands 
            (or wait a few minutes for the port to clear again):
        bin/poker_server 10081
        bin/poker_client serverName 10081
* Multiple players can log on, but they must log on one at a time.  If multiple players have the login screen open, 
                the info they will have will be out of date (the program works by sending login data once to the client when they open the program, and then the client sends the updated version back to the server; the server can then send this updated version to the next client who connects, and so on)
* The user must use -X when logging onto the EECS server for the GTK display to work.

**********************************************************
Copyright

    Copyright © 2026 All In.  All rights reserved.
    The unauthorized sale or distribution of this software is prohibited.
    
Disclaimer of Warranty

    THIS SOFTWARE IS PROVIDED “AS IS” WITHOUT ANY EXPRESS OR IMPLIED WARRANTY OF ANY KIND.
    