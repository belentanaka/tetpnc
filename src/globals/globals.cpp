#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>

#include "../constants.h"
#include "globals.h"

// High scores
Score gScores[ TOTAL_SCORES ];

// SDL objects for rendering
SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font* gFont = NULL; // Font

// Menu music
Mix_Chunk* gStartMusic = NULL;
Mix_Chunk* gScoreMusic = NULL;

// Sound effects
Mix_Chunk* gMoveSound = NULL;
Mix_Chunk* gHoldSound = NULL;
Mix_Chunk* gLandSound = NULL;
Mix_Chunk* gClearSound = NULL;
Mix_Chunk* gTetrisSound = NULL;
Mix_Chunk* gGameOverSound = NULL;

// Background music
Mix_Chunk* gBGMusic[ TOTAL_BGM ];
