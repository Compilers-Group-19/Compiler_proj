CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -static
SRC      := $(wildcard src/*.cpp)
HDR      := $(wildcard src/*.h)
TARGET   := scanner

all: $(TARGET)

$(TARGET): $(SRC) $(HDR)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC)

test: $(TARGET)
	./$(TARGET) tests/sample.mini
	-./$(TARGET) tests/errors.mini

clean:
	rm -f $(TARGET) $(TARGET).exe

.PHONY: all test clean
