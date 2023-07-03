CXX = g++
CXXFLAGS = -Wall -Wextra -Werror

all: decoder

decoder: Decoder.cpp
	$(CXX) $(CXXFLAGS) -o decoder Decoder.cpp

run: decoder
	./decoder

clean:
	rm -f decoder
