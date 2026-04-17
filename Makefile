# Variables
CC = gcc
CFLAGS = -I./include -I./motor -Wall
LDFLAGS = -L./lib -lSDL3 -lm # -mwindows
OBJ = motor.exe

# Regla principal
all:
	$(CC) ./src/main.c -o $(OBJ) $(CFLAGS) $(LDFLAGS)