CC = gcc
CXX = g++

CFLAGS = -Wall -Wextra -Iinclude
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude

GAME_LIBS = -lncurses
TEST_LIBS = -lgtest -lgtest_main -pthread -lncurses

.PHONY: all run test clean

all: Racing_game Tests

racing_game.o: src/racing_game.c include/racing_game.h
	$(CC) $(CFLAGS) -c src/racing_game.c -o racing_game.o

Racing_game: demo/main.c racing_game.o
	$(CC) $(CFLAGS) demo/main.c racing_game.o -o Racing_game $(GAME_LIBS)

Tests: tests/test_racing_game.cpp racing_game.o
	$(CXX) $(CXXFLAGS) tests/test_racing_game.cpp racing_game.o -o Tests $(TEST_LIBS)

run: Racing_game
	./Racing_game

test: Tests
	./Tests

clean:
	rm -f Racing_game Tests racing_game.o