CXX := g++
CXXFLAGS := -I./include -g -std=c++20 -O0 -fno-inline-functions
LDLIBS := -lfmt
files := main.cpp conf.cpp io.cpp

all: main

main: $(files)

.PHONY: clean

clean:
	rm main
