OBJS = src/globals/globals.cpp src/LTexture/LTexture.cpp src/textures/textures.cpp src/Timer/Timer.cpp src/Square/Square.cpp src/Tetromino/Tetromino.cpp src/GameState/GameState.cpp src/Intro/Intro.cpp src/Play/Play.cpp src/GameOver/GameOver.cpp src/ScoreList/ScoreList.cpp src/functions/functions.cpp src/main.cpp

CC = g++

COMPILER_FLAGS = -w

LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

OBJ_NAME = tetpnc

all : $(OBJS)
	if [ ! -d bin ]; then mkdir bin; fi
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)

clean : 
	-rm $(OBJ_NAME)