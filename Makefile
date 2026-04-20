# Variables
CC = gcc
CFLAGS = -I./include -I./motor -Wall
LDFLAGS = -L./lib -lSDL3 -lm # -mwindows
OBJ = motor.exe
SRC = $(wildcard src/*.c) $(wildcard src/*/*.c)

# Regla principal
all:
	$(CC) $(SRC) -o $(OBJ) $(CFLAGS) $(LDFLAGS)