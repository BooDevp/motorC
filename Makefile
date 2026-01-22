# Compilador y Flags
CC = gcc
# Añadimos -I./include para que encuentre SDL3/SDL.h
CFLAGS = -I./include -Wall -std=c11
# Añadimos -L./lib y -lopengl32 para que NVIDIA pueda hablar con C
LDFLAGS = -L./lib -lSDL3 -lopengl32 -lm

# BUSQUEDA AUTOMÁTICA:
SRC = $(wildcard src/*.c)
TARGET = motor.exe

# REGLA PRINCIPAL
# Eliminamos la dependencia de glslangValidator porque OpenGL usa texto plano (.vert/.frag)
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LDFLAGS)

clean:
	del /q $(TARGET)
