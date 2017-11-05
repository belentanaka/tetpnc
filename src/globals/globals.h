#ifndef GLOBALS_H
#define GLOBALS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include "../constants.h"

// High score data
struct Score
{
  char name[ 11 ];
  int score;
};

// High scores
extern Score gScores[ TOTAL_SCORES ];

// Game stats
struct Stats
{
  TetrominoFlag nextTetrominoes[ 3 ];
  TetrominoFlag holdTetromino;
  int score;
  int lines;
  int level;
  int currentBG;
  int currentBGM;
};  

// SDL objects for rendering
extern SDL_Window* gWindow;
extern SDL_Surface* gScreenSurface;
extern SDL_Renderer* gRenderer;
extern TTF_Font* gFont; // Font

// Menu music
extern Mix_Chunk* gStartMusic;
extern Mix_Chunk* gScoreMusic;

// Sound effects
extern Mix_Chunk* gMoveSound;
extern Mix_Chunk* gHoldSound;
extern Mix_Chunk* gLandSound;
extern Mix_Chunk* gClearSound;
extern Mix_Chunk* gTetrisSound;
extern Mix_Chunk* gGameOverSound;

// Background music
extern Mix_Chunk* gBGMusic[ TOTAL_BGM ];

#endif
