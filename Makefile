# Makefile
SHARED = cards.hpp data.hpp
OBJ = g++ -c -Wall

clean:
	rm -f *.o

test:  testCards testData

# Main executables


# Main object files
cards.o:  cards.cpp cards.hpp
	$(OBJ) cards.cpp -o cards.o


# Testing executables
testCards:  testCards.o cards.o
	g++ -Wall testCards.o cards.o -o testCards

testData:  testData.o cards.o
	g++ -Wall testData.o cards.o -o testData


# Testing object files
testCards.o:  testCards.cpp $(SHARED)
	$(OBJ) testCards.cpp -o testCards.o

testData.o:  testData.cpp $(SHARED)
	$(OBJ) testData.cpp -o testData.o
