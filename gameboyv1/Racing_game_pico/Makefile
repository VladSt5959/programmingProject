CC = gcc
CXX = g++

CFLAGS = -Wall -Wextra -Iinclude
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude
TEST_LIBS = -lgtest -lgtest_main -pthread

.PHONY: all test clean

all: Tests

racing_game.o: src/racing_game.c include/racing_game.h
	$(CC) $(CFLAGS) -c src/racing_game.c -o racing_game.o

Tests: tests/test_racing_game.cpp racing_game.o
	$(CXX) $(CXXFLAGS) tests/test_racing_game.cpp racing_game.o -o Tests $(TEST_LIBS)

test: Tests
	./Tests

clean:
	rm -f Tests racing_game.o