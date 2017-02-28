CXX := g++
CXXFLAGS += -pipe
CXXFLAGS += -O3 -flto -flto-odr-type-merging
CXXFLAGS += -Wall -Wextra

.PHONY: all clean install

all: gcc2vs.exe

gcc2vs.exe: gcc2vs.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	-rm -f gcc2vs.exe

install: gcc2vs.exe
	cp -f gcc2vs.exe /usr/bin/
