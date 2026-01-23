# ============================================================================
# MOTOR GRÁFICO CON OPENGL 3.3 Y SDL3 - WINDOWS
# ============================================================================

# Compilador y Flags
CC = gcc
CFLAGS = -I./include -Wall -Wextra -std=c11 -g
LDFLAGS = -L./lib -lSDL3 -lopengl32 -lm

# Archivos fuente
SRC = src/main.c
TARGET = motor.exe

# Regla principal
all: $(TARGET)

$(TARGET): $(SRC)
	@echo ========================================
	@echo COMPILANDO MOTOR OPENGL 3.3 Windows
	@echo ========================================
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LDFLAGS)
	@echo Compilación completada: $(TARGET)
	@echo Ejecuta: $(TARGET)

# Limpieza
clean:
	@echo Limpiando proyecto...
	del /q $(TARGET) 2>nul || true

run: $(TARGET)
	$(TARGET)

.PHONY: all clean run