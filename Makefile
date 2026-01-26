CC = gcc
# -I./include para el motor y SDL3
# -I./src para las escenas
CFLAGS = -I./include -I./src -Wall -Wextra -std=c11 -g
LDFLAGS = -L./lib -lSDL3 -lopengl32 -lm

SRC = src/main.c
TARGET = motor.exe

.PHONY: all clean run

all:
	@echo Compilando: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)
	@echo ¡Motor listo!

clean:
	del /q $(TARGET) 2>nul || true

run: all
	.\$(TARGET)