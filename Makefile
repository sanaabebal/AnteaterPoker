# Makefile
CC = g++
CFLAGS = -Wall -g $(shell pkg-config --cflags gtk+-3.0)
LIBS = $(shell pkg-config --libs gtk+-3.0)

# Tester config parameters for port and host
PORT = 10080
HOST = 127.0.0.1

SHAREDSRCS = src/cards.cpp src/DataTransfer.cpp src/stubFunctionsW9.cpp

# compiling all the shared sources
SHAREDOBJS = $(SHAREDSRCS:.cpp=.o)

all:  ./bin/poker_client ./bin/poker_server
	@echo ""
	@echo ""
	@echo "File names:  bin/poker_server bin/poker_client"

bin/poker_client: src/clientGUI.o $(SHAREDOBJS)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o bin/poker_client src/clientGUI.o $(SHAREDOBJS) $(LIBS)

bin/poker_server: src/serverGUI.o $(SHAREDOBJS)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o bin/poker_server src/serverGUI.o $(SHAREDOBJS) $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

test_server: bin/poker_server
	@echo ""
	@echo ""
	@echo "Starting up game server while on port $(PORT)..."
	./bin/poker_server $(PORT)

test_client: $(SHAREDOBJS)
	@echo ""
	@echo ""
	@echo "Compiling and terminal-based client for validation..."
	$(CC) -Wall -g -c src/clientTextW8.cpp -o src/clientTextW8.o
	$(CC) -Wall -g -o bin/test_client_bin src/clientTextW8.o $(SHAREDOBJS)
	./bin/test_client_bin $(HOST) $(PORT)

test_gui: bin/poker_client
	@echo ""
	@echo ""
	@echo "Launching up GUI for poker client while linking to $(HOST) $(PORT)..."
	./bin/poker_client $(HOST) $(PORT)

tar: clean all
	@echo ""
	@echo ""
	@echo "Packaging for final release"

	mkdir -p poker/bin poker/doc poker_src/src poker_src/assets

	# User Packaged Content
	# in case a file is missing, compiling will halt
	cp README COPYRIGHT INSTALL poker/ 2>/dev/null || true
	cp bin/poker_client bin/poker_server poker/bin/
	cp Poker_UserManual.pdf poker/doc/
	gtar czf Poker_V1.0.tar.gz poker/

	# Source Code Packaged Content
	cp README COPYRIGHT INSTALL Makefile Poker_UserManual.pdf Poker_SoftwareSpec.pdf poker_src/
	cp src/* poker_src/src/
	cp -r assets/* poker_src/assets/ 2>/dev/null || true
	gtar czf Poker_V1.0_src.tar.gz poker_src/

	#removing extra directories
	rm -rf poker poker_src
	@echo ""
	@echo ""
	@echo "Packaged together Poker_V1.0.tar.gz and Poker_V1.0_src.tar.gz successfully"

clean:
	rm -r src/*.o bin/*
	rm -rf bin

.PHONY: all clean test_server test_client test_gui tar
