PKG_CFLAGS := $(shell pkg-config --cflags libdpdk spdlog)
PKG_LIBS := $(shell pkg-config --libs libdpdk spdlog)

# Compiler
CXX = g++
CXXFLAGS = -O2 -Wall -std=c++17 $(PKG_CFLAGS)
LDFLAGS = -lpthread -lnuma -ldl $(PKG_LIBS)

SRC_DIR := ./
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SRCS:.cpp=.o)
TARGET := bypass

all: $(TARGET)

# Link
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# cpp->o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# clean
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
