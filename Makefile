project 	:= cpputil
files 		:= main.cpp conf.cpp io.cpp
platform 	:= linux
buildtype 	:= debug
CC 			:= gcc
CXX 		:= g++
CFLAGS 		:= -I./external/include -std=c11 -Wall -Wextra -pedantic
CXXFLAGS 	:= -I./external/include -std=c++20 -Wall -Wextra -pedantic
LDLIBS 		:= -lfmt

VPATH := src
outdir := debug
flags_deps = -MMD -MP -MF $(@:.o=.d)
ifeq ($(buildtype),debug)
    outdir := debug
    CFLAGS += -g -O0 -fno-inline-functions -DDEBUG
    CXXFLAGS += -g -O0 -fno-inline-functions -DDEBUG
else ifeq ($(buildtype),release)
    outdir := release
    CFLAGS += -O3 -DNDEBUG
    CXXFLAGS += -O3 -DNDEBUG
else
	$(error error: invalid value for buildtype)
endif

objs := $(patsubst %,$(outdir)/%.o,$(files))

all: $(outdir)/$(project)

$(outdir)/$(project): $(outdir) $(objs)
	$(CXX) $(objs) -o $@ $(LDLIBS)

-include $(outdir)/*.d

$(outdir)/%.cpp.o: %.cpp
	$(CXX) $(CXXFLAGS) $(flags_deps) -c $< -o $@

$(outdir)/%.c.o: %.c
	$(CC) $(CFLAGS) $(flags_deps) -c $< -o $@

$(outdir):
	mkdir -p $(outdir)

clean:
	rm -rf $(outdir)

.PHONY: clean
