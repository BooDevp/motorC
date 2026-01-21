# Compilador y Flags
CC = gcc
CFLAGS = -I./include -Wall
LDFLAGS = -L./lib -lSDL3 -lm

# BUSQUEDA AUTOMÁTICA:
# Busca todos los .c en la carpeta src
SRC = $(wildcard src/*.c)
# Crea los nombres de los .exe a partir de los .c (opcional, pero útil para depurar)
OBJ_NAMES = $(SRC:.c=.o)
TARGET = motor.exe

# SHADERS AUTOMÁTICOS:
GLSLANG = ./glslangValidator.exe
# Busca todos los archivos de shader
VERT_SOURCES = $(wildcard src/shaders/*.vert)
FRAG_SOURCES = $(wildcard src/shaders/*.frag)
# Define que por cada .vert y .frag debe existir un .spv
SHADER_BINARIES = $(VERT_SOURCES:.vert=.vert.spv) $(FRAG_SOURCES:.frag=.frag.spv)

# REGLA PRINCIPAL
# Ahora depende de la lista automática de shaders
all: $(SHADER_BINARIES)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LDFLAGS)

# REGLA PARA COMPILAR CUALQUIER VERTEX SHADER
%.vert.spv: %.vert
	$(GLSLANG) -V $< -o $@

# REGLA PARA COMPILAR CUALQUIER FRAGMENT SHADER
%.frag.spv: %.frag
	$(GLSLANG) -V $< -o $@

clean:
	del /q $(TARGET) src\shaders\*.spv