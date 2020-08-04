CXX := g++
CXXFLAGS += -O3 -flto -std=gnu++17
CXXFLAGS += -Wall -Wextra

.PHONY: all clean install uninstall

all: gcc2vs.exe

gcc2vs.exe: gcc2vs.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	-rm -f gcc2vs.exe

install: gcc2vs.exe
	mkdir -p /usr/bin
	cp -f gcc2vs.exe /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/gcc2vs.exe
