CXX = g++
CXXFLAGS = -Iinc -Wall -std=c++17

SRC = src/main.cpp \
	  src/tui.cpp
OUT = out/app

$(OUT): $(SRC)
	$(CXX) $(SRC) $(CXXFLAGS) -o $(OUT)

clean:
	rm -f $(OUT)