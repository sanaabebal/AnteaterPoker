CXX = g++
CXXFLAGS = -Wall -g `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0`

TARGET = test
# CHANGED: Added winning_hand.cpp to the source files list
SRC = test.cpp test_winningHand.cpp login.cpp

$(TARGET) : $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)
