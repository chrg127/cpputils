CXX := g++
CXXFLAGS := -I./include -g -std=c++20
LDLIBS := -lfmt
files := main.cpp conf.cpp

all: main

main: $(files)

.PHONY: clean

clean:
	rm main
