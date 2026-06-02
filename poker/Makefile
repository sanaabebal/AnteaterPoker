# Makefile
SHARED = ./src/cards.hpp ./src/data.hpp
OBJ = g++ -c -Wall

GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS = $(shell pkg-config --libs gtk+-3.0)

all:  ./bin/poker_client ./bin/poker_server
	@echo ""
	@echo ""
	@echo "File names:  bin/poker_server bin/poker_client"

# @echo ""
# @echo ""
# @echo "----------PLEASE RUN bin/poker_client serverName 10080 IF YOU ARE NOT USING THE BONDI SERVER.  You will be prompted to choose via text interface if you do not.----------"

clean:
	rm -f *.o

test:  ./bin/testCards ./bin/testData ./bin/testAlphaServer ./bin/testAlphaClient ./bin/testAlphaClientGUI ./bin/testGUIW9 ./bin/testClientCommW9 ./bin/testServerCommW9
	@echo ""
	@echo ""
	@echo "Testing file names:  bin/testCards bin/testData bin/testAlphaServer bin/testAlphaClient bin/testAlphaClientGUI bin/testClientCommW9 bin/testServerCommW9 bin/testClientGUI"
	bin/testData
	bin/testGUIW9
	bin/testServerCommW9 &
	bin/testClientCommW9

test-gui test_gui:  ./bin/testGUIW9
	@echo ""
	@echo ""
	@echo "Testing client GUI for game"
	./bin/testGUIW9

# @echo "---------PLEASE RUN bin/testAlphaClientGUI serverName 10080 IF YOU ARE NOT USING THE BONDI SERVER--------"
# ./bin/testAlphaServer 10080

test-comm test_comm:  bin/testServerCommW9 bin/testClientCommW9
	bin/testServerCommW9 &
	hostname -I
	bin/testClientCommW9

test-server test_server:  bin/testAlphaServer
	@echo ""
	@echo ""
	@echo "Testing server"
	bin/testAlphaServer &

test-client test_client:  bin/testAlphaClient
	@echo ""
	@echo ""
	@echo "Testing client:  run bin/testAlphaClient hostname 10080"
	hostname -I

LHTEST:  ./bin/testGUIW9


# Main executables
./bin/poker_server:  serverGUIW9.o cards.o DataTransfer.o gamestate.o
	g++ -Wall serverGUIW9.o cards.o DataTransfer.o gamestate.o -o ./bin/poker_server $(GTK_LIBS)

./bin/poker_client:  clientGUIW9.o cards.o DataTransfer.o gamestate.o
	g++ -Wall clientGUIW9.o cards.o DataTransfer.o gamestate.o -o ./bin/poker_client $(GTK_LIBS)



# Main object files
cards.o:  ./src/cards.cpp ./src/cards.hpp
	$(OBJ) ./src/cards.cpp -o cards.o



# Incorrectly named/organized object files  (will be addressed in later versions)
serverGUIW8.o:  ./src/serverGUIW8.cpp $(SHARED)
	g++ -c $(GTK_CFLAGS) ./src/serverGUIW8.cpp -o serverGUIW8.o $(pkg-config --cflags --libs gtk+-3.0)

DataTransfer.o:  ./src/DataTransfer.cpp ./src/DataTransfer.hpp $(SHARED)
	g++ -c ./src/DataTransfer.cpp -o DataTransfer.o

clientTextW8.o:  ./src/clientTextW8.cpp $(SHARED)
	$(OBJ) ./src/clientTextW8.cpp -o clientTextW8.o

clientGUIW8.o:  ./src/clientGUIW8.cpp $(SHARED)
	g++ -c $(GTK_CFLAGS) ./src/clientGUIW8.cpp -o clientGUIW8.o

serverGUIW9.o:  ./src/serverGUIW9.cpp $(SHARED)
	g++ -c $(GTK_CFLAGS) ./src/serverGUIW9.cpp -o serverGUIW9.o $(pkg-config --cflags --libs gtk+-3.0)

clientGUIW9.o:  ./src/clientGUIW9.cpp $(SHARED)
	g++ -c $(GTK_CFLAGS) ./src/clientGUIW9.cpp -o clientGUIW9.o

stubFunctionsW9.o:  ./src/stubFunctionsW9.cpp $(SHARED)
	g++ -c $(GTK_CFLAGS) ./src/stubFunctionsW9.cpp -o stubFunctionsW9.o

gamestate.o:  ./src/gamestate.cpp $(SHARED) ./src/gamestate.hpp
	g++ -c $(GTK_CFLAGS) ./src/gamestate.cpp -o gamestate.o




# Testing executables
./bin/testCards:  testCards.o cards.o
	g++ -Wall testCards.o cards.o -o ./bin/testCards

./bin/testData:  testData.o cards.o
	g++ -Wall testData.o cards.o -o ./bin/testData

./bin/testAlphaServer:  serverGUIW8.o cards.o DataTransfer.o
	g++ -Wall serverGUIW8.o cards.o DataTransfer.o -o ./bin/testAlphaServer $(GTK_LIBS)

./bin/testAlphaClient:  clientTextW8.o cards.o DataTransfer.o
	g++ -Wall clientTextW8.o cards.o DataTransfer.o -o ./bin/testAlphaClient

./bin/testAlphaClientGUI:  clientGUIW8.o cards.o DataTransfer.o
	g++ -Wall clientGUIW8.o cards.o DataTransfer.o -o ./bin/testAlphaClientGUI $(GTK_LIBS)

./bin/testGUIW9:  src/testClientGUI.cpp $(SHARED)
	g++ -Wall src/testClientGUI.cpp -o ./bin/testGUIW9 $(GTK_LIBS) $(GTK_CFLAGS)

./bin/testServerCommW9:  testServerCommW9.o cards.o DataTransfer.o
	g++ -Wall testServerCommW9.o cards.o DataTransfer.o -o ./bin/testServerCommW9 $(GTK_LIBS)

./bin/testClientCommW9:  testClientCommW9.o cards.o DataTransfer.o
	g++ -Wall testClientCommW9.o cards.o DataTransfer.o -o ./bin/testClientCommW9 $(GTK_LIBS)

# Testing object files
testCards.o:  ./src/testCards.cpp $(SHARED)
	$(OBJ) ./src/testCards.cpp -o testCards.o

testData.o:  ./src/testData.cpp $(SHARED)
	$(OBJ) ./src/testData.cpp -o testData.o

testServerCommW9.o:  ./src/testServerCommW9.cpp $(SHARED) src/DataTransfer.hpp
	$(OBJ) ./src/testServerCommW9.cpp -o testServerCommW9.o $(GTK_CFLAGS)

testClientCommW9.o:  ./src/testClientCommW9.cpp $(SHARED) src/DataTransfer.hpp
	$(OBJ) ./src/testClientCommW9.cpp -o testClientCommW9.o $(GTK_CFLAGS)




# Tar instructions
tar:  
	tar -zcvf Poker_Beta.tar.gz ./README ./COPYRIGHT ./INSTALL ./bin ./doc/Poker_UserManual.pdf
	tar -zcvf Poker_Beta_src.tar.gz ./README ./COPYRIGHT ./INSTALL ./assets ./doc ./bin ./src ./Makefile