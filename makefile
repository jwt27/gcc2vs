CXX := g++
CXXFLAGS += -O3 -flto
CXXFLAGS += -Wall -Wextra

.PHONY: all clean install

all: gcc2vs.exe

gcc2vs.exe: gcc2vs.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	-rm -f gcc2vs.exe

install: gcc2vs.exe
	cp -f gcc2vs.exe /usr/bin/
