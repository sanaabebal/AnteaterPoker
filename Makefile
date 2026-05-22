# Makefile
SHARED = cards.hpp data.hpp
OBJ = g++ -c -Wall

GTK_CFLAGS = $(shell pkg-config --cflags gtk+-2.0)
GTK_LIBS = $(shell pkg-config --libs gtk+-2.0)

clean:
	rm -f *.o

test:  testCards testData testAlpha

# Main executables


# Main object files
cards.o:  cards.cpp cards.hpp
	$(OBJ) cards.cpp -o cards.o


# Testing executables
testCards:  testCards.o cards.o
	g++ -Wall testCards.o cards.o -o testCards

testData:  testData.o cards.o
	g++ -Wall testData.o cards.o -o testData

testAlphaServer:  serverGUIW8.o cards.o DataTransfer.o
	g++ -Wall serverGUIW8.o cards.o DataTransfer.o -o testAlphaServer $(GTK_LIBS)

testAlphaClient:  clientTextW8.o cards.o DataTransfer.o
	g++ -Wall clientTextW8.o cards.o DataTransfer.o -o testAlphaClient


# Testing object files
testCards.o:  testCards.cpp $(SHARED)
	$(OBJ) testCards.cpp -o testCards.o

testData.o:  testData.cpp $(SHARED)
	$(OBJ) testData.cpp -o testData.o


serverGUIW8.o:  serverGUIW8.cpp $(SHARED)
	g++ -c $(GTK_CFLAGS) serverGUIW8.cpp -o serverGUIW8.o $(pkg-config --cflags --libs gtk+-2.0)

DataTransfer.o:  DataTransfer.cpp DataTransfer.hpp $(SHARED)
	g++ -c DataTransfer.cpp -o DataTransfer.o

clientTextW8.o:  clientTextW8.cpp $(SHARED)
	$(OBJ) clientTextW8.cpp -o clientTextW8.o