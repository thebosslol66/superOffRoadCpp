CXX = clang++
CXXFLAGS = -Wall -g -O2 -std=c++11
LDFLAGS = -lsfml-system -lsfml-window -lsfml-graphics -lsfml-audio

TARGETS = game

all: $(TARGETS)

clean:
	rm -f $(TARGETS)
