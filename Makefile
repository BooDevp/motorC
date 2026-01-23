# ============================================================================
# MOTOR GRÁFICO CON SDL3 - CONFIGURACIÓN AUTOMÁTICA
# ============================================================================

# Compilador y Flags
CC = gcc
CFLAGS = -I./include -Wall -Wextra -std=c11 -g
LDFLAGS = -L./lib -lSDL3 -lopengl32 -lm

# 1. BUSCAR ARCHIVOS AUTOMÁTICAMENTE
# Buscamos todos los archivos .c dentro de la carpeta src
SRC = $(wildcard src/*.c)

# 2. GENERAR LISTA DE OBJETOS (.o)
# Esto convierte, por ejemplo, src/main.c en src/main.o
OBJ = $(SRC:.c=.o)

TARGET = motor.exe

# Regla principal
all: $(TARGET)

# Linkado final
$(TARGET): $(OBJ)
	@echo ========================================
	@echo LINKANDO: $@
	@echo ========================================
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)
	@echo Compilacion completada.

# Regla para compilar cada archivo .c en un .o
# Se activa automáticamente si el .c es más nuevo que el .o
%.o: %.c
	@echo Compilando fuente: $<
	$(CC) $(CFLAGS) -c $< -o $@

# Limpieza (Windows)
clean:
	@echo Limpiando proyecto...
	del /q src\*.o $(TARGET) 2>nul || true

run: $(TARGET)
	@echo Ejecutando...
	.\$(TARGET)

.PHONY: all clean run