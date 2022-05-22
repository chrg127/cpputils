CXX := g++
CXXFLAGS := -g -std=c++20
LDLIBS := -lfmt
files := main.cpp conf.cpp

all: main

main: $(files)
