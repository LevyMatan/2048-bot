CXX = clang++
CXXFLAGS = -O3 -Wall -Wextra -std=c++17 -stdlib=libc++ \
           -isysroot $(shell xcrun --show-sdk-path) \
           -I$(shell xcrun --show-sdk-path)/usr/include/c++/v1

LDFLAGS = -stdlib=libc++

SRCS = main.cpp board.cpp game.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = 2048

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)