CXX = g++
CXXFLAGS = -g

all: decoder

decoder: Decoder.cpp jpeg_data.cpp
	$(CXX) $(CXXFLAGS) -o decoder Decoder.cpp jpeg_data.cpp

run: decoder
	./decoder

clean:
	rm -f decoder
