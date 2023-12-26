.SUFFIXES:

project 	:= cpputil
files 		:= conf.cpp io.cpp
main_files	:=
test_files	:= array_test.cpp bits_test.cpp callcommand_test.cpp \
			   cmdline_test.cpp conf_test.cpp io_test.cpp string_test.cpp \
			   flags_test.cpp math_test.cpp
test_name	:= test
conf_example_files := conf_example.cpp
conf_example_name  := conf_example
io_example_files 	:= io_example.cpp
io_example_name		:= io_example
platform 	:= linux
buildtype 	:= debug
CC 			:= gcc
CXX 		:= g++
CFLAGS 		:= -I./external/include -std=c11 -Wall -Wextra -pedantic
CXXFLAGS 	:= -I./external/include -std=c++20 -Wall -Wextra -pedantic \
			   $(shell pkg-config --cflags catch2-with-main)
LDLIBS 		:= -lfmt
libs_test 	:= $(shell pkg-config --libs catch2-with-main)
PREFIX		:= /usr/local
DESTDIR		:=
VPATH 		:= src:test:examples

outdir := debug
ifeq ($(buildtype),debug)
    outdir := debug
    CFLAGS += -g -O0 -fno-inline-functions -DDEBUG
    CXXFLAGS += -g -O0 -fno-inline-functions -DDEBUG
else ifeq ($(buildtype),release)
    outdir := release
    CFLAGS += -O3 -DNDEBUG
    CXXFLAGS += -O3 -DNDEBUG
else ifeq ($(buildtype),reldeb)
	outdir := reldeb
	CFLAGS += -g -Og -DDEBUG
	CXXFLAGS += -g -Og -DDEBUG
else
	$(error error: invalid value for buildtype)
endif

flags_deps 	= -MMD -MP -MF $(@:.o=.d)
objs 		:= $(patsubst %,$(outdir)/%.o,$(files))
objs_main	:= $(patsubst %,$(outdir)/%.o,$(main_files))
objs_test 	:= $(patsubst %,$(outdir)/%.o,$(test_files))
objs_conf_example	:= $(patsubst %,$(outdir)/%.o,$(conf_example_files))
objs_io_example		:= $(patsubst %,$(outdir)/%.o,$(io_example_files))

all: $(outdir)/$(project)

test: $(outdir)/$(test_name)

conf_example: $(outdir)/$(conf_example_name)

io_example: $(outdir)/$(io_example_name)

install:

uninstall:

clean:
	rm -rf $(outdir)

$(outdir)/$(project): $(outdir) $(objs) $(objs_main)
	$(CXX) $(objs) $(objs_main) -o $@ $(LDLIBS)

$(outdir)/$(test_name): $(outdir) $(objs) $(objs_test)
	$(CXX) $(objs) $(objs_test) -o $@ $(LDLIBS) $(libs_test)

$(outdir)/$(conf_example_name): $(outdir) $(objs) $(objs_conf_example)
	$(CXX) $(objs) $(objs_conf_example) -o $@ $(LDLIBS)

$(outdir)/$(io_example_name): $(outdir) $(objs) $(objs_io_example)
	$(CXX) $(objs) $(objs_io_example) -o $@ $(LDLIBS)

$(outdir):
	mkdir -p $(outdir)

-include $(outdir)/*.d

$(outdir)/%.cpp.o: %.cpp
	$(CXX) $(CXXFLAGS) $(flags_deps) -c $< -o $@

$(outdir)/%.c.o: %.c
	$(CC) $(CFLAGS) $(flags_deps) -c $< -o $@

.PHONY: clean install uninstall
