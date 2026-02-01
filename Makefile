CC = gcc

# Directorios de cabeceras
# Asegúrate de que sokol_gfx.h esté en ./include/external o ajusta la ruta
CFLAGS = -I./include -I./src -Wall -Wextra -std=c11 -g

# Librerías
# Añadimos gdi32 y shell32 porque Sokol en Windows las usa para el backend de OpenGL
LDFLAGS = -L./lib -lSDL3 -lopengl32 -lgdi32 -lshell32 -lm

# Si en el futuro separas el código en varios archivos .c, 
# puedes usar SRC = $(wildcard src/*.c)
SRC = src/main.c
TARGET = motor.exe

.PHONY: all clean run

all:
	@echo ========================================
	@echo Compilando Motor Sokol: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)
	@echo ¡Motor listo para la acción!
	@echo ========================================

clean:
	@echo Limpiando...
	@if exist $(TARGET) del /q $(TARGET)

run: all
	.\$(TARGET)