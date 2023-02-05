CXXFLAGS = -O3 -std=c++17 -pedantic -Wall -Werror -Wextra
LDFLAGS =

HDR = cyclic_shared.hpp
FWD = $(HDR:.hpp=.fwd.hpp)
SRC = $(HDR:.hpp=.cpp) test.cpp
OBJ = $(SRC:.cpp=.o)
BIN = test

all: $(BIN)

test: $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $+

.c.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJ): $(HDR) $(FWD)

clean:
	rm -fr $(OBJ) $(BIN)

.PHONY: all clean
