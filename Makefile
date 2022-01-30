CXX = g++
CXXFLAGS = -Wall -Wpedantic -std=c++20
LDFLAGS = -lSDL2 -lm

imageViewer: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o imageViewer $(LDFLAGS)

debug: main.cpp
	$(CXX) $(CXXFLAGS) -g main.cpp -o imageViewer $(LDFLAGS)

clean:
	rm -rf imageViewer
