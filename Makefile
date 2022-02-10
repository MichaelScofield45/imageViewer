CXX = g++
CXXFLAGS = -Wall -Wpedantic -std=c++20
LDFLAGS = -lSDL2

imageViewer: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o imageViewer $(LDFLAGS)

debug: main.cpp
	$(CXX) $(CXXFLAGS) -g main.cpp -o imageViewer $(LDFLAGS)

release: main.cpp
	$(CXX) $(CXXFLAGS) -O2 main.cpp -o imageViewer $(LDFLAGS)

clean:
	rm -rf imageViewer
