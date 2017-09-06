OBJS = tetpnc.cpp

CC = g++

COMPILER_FLAGS = -w

LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

OBJ_NAME = tetpnc

all : $(OBJS)
	if [ ! -d bin ]; then mkdir bin; fi
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)

clean : 
	-rm $(OBJ_NAME)