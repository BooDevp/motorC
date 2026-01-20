# Variables
CC = gcc
CFLAGS = -I./include -I./motor -Wall
LDFLAGS = -L./lib -lSDL3 -lm # -mwindows
SRC = $(shell find . -name "*.c") # Busca todo automáticamente
OBJ = motor.exe

# Regla principal
all:
	$(CC) $(SRC) -o $(OBJ) $(CFLAGS) $(LDFLAGS)