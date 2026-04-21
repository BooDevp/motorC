# Variables
CC = gcc
CFLAGS = -I./include -I./motor -Wall
LDFLAGS = -L./lib -lSDL3 -lm # -mwindows
OBJ = motor.exe
SRC = $(shell find src -name "*.o" -prune -o -name "*.c" -print)

# Regla principal
all:
	$(CC) $(SRC) -o $(OBJ) $(CFLAGS) $(LDFLAGS)