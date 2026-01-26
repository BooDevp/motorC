# ============================================================================
# MOTOR GRÁFICO CON SDL3 - SIEMPRE RECOMPILAR
# ============================================================================

CC = gcc
CFLAGS = -I./include -Wall -Wextra -std=c11 -g
LDFLAGS = -L./lib -lSDL3 -lopengl32 -lm

# Buscamos todos los archivos .c
SRC = $(wildcard src/*.c)
TARGET = motor.exe

# Marcamos 'all' como PHONY para que ignore si los archivos son viejos o nuevos
# y siempre ejecute la receta.
.PHONY: all clean run

all:
	@echo ========================================
	@echo COMPILANDO TODO EL PROYECTO...
	@echo ========================================
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)
	@echo Compilacion completada con exito.

clean:
	@echo Limpiando proyecto...
	del /q $(TARGET) 2>nul || true

run: all
	@echo Ejecutando...
	.\$(TARGET)