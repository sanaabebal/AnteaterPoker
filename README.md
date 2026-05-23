README.md

LH V2.1.01:  Text client, GUI server seem to be working

To-Do (Must be fixed before submission):
    * Still relying on command-line arguments...most likely the best option is to adjust it so 
        that if there are not enough command-line arguments (like in the required submission specs),
        defaults will be chosen that match one of the port numbers, servers we were given 
        (be sure to refer to slides to make sure we pick only our assigned port numbers)
        (also, please refrain from choosing a server that requires campus presence/VPN to use)
            * Defaults probably best b/c there were some cases where server was a little finnicky about 
                shutting down, and it would probably be best to allow the user to override and pick another port
                in the event that one doesn't clear out correctly
    * Client GUI doesn't exist
        * Hopefully it has been modularized correctly so this can easily be changed (see clientTextW8.cpp)?
        * Think the only thing really missing is the waiting screen/exit screen (right now, I doubt we can do much more than this before the deadline)
    * Makefile needs to be adjusted so that it creates files with the correct names
    * Putting files in the right place, putting in copyright/full readme/documentation, etc.

    * There are other things I may be missing - please check!

Other Notes:
    * Technically the file names are wrong and all of DataTransfer needs to be moved, but for now I found it convenient to not have to put the same code
        in two different files


Run Notes:
    * The program is intended to have the user connect to the server to get the information needed for logging in, 
        allow the player to go through the login menu based on whether or not they are the host,
        and send that information from the login menu to the server, which will then be reflected on the server GUI.
        The player will then be sent to a waiting screen/exit screen, as this is as far as the program has gone at this point.
    * NOTE:  In the event that the server is refusing to initialize because it says the port is already in use, please use the following commands:
        _________
        _________
    * NOTE:  Multiple players can log on, but they must log on one at a time.  If multiple players have the login screen open, the info they will have will be out of date
                (the program works by sending login data once to the client when they open the program, and then the client sends the updated version back to the server;
                the server can then send this updated version to the next client who connects, and so on)
    * NOTE:  The user MUST use -X when logging onto the EECS server for the GTK display to work.
    