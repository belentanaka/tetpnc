#include <SDL2/SDL.h>

#include "../constants.h"
#include "../LTexture/LTexture.h"
#include "textures.h"

// Menu/Gameplay textures
LTexture gBlankBGTexture;
LTexture gPressEnterTexture;
LTexture gPlayBGTexture;
LTexture gPausedTexture;
LTexture gGameOverTexture;
LTexture gEnterNameTexture;
LTexture gBlackTexture;
LTexture gTetrisTexture;

// Background textures
LTexture gBGTextures[ TOTAL_BG ];

// Hand textures and animation clips
LTexture gHandBlackTexture;
LTexture gHandWhiteTexture;
SDL_Rect gHandClips[ 5 ];

// High score textures and clips
LTexture gListTexture;
SDL_Rect gListClips[ TOTAL_SCORES ];

// Square sprites and clips 
LTexture gSquareSpriteTexture;
SDL_Rect gSquareSpriteClips[ SQUARE_SPRITE_TOTAL ];

// Text textures
LTexture gScoreTextTexture;
LTexture gLinesTextTexture;
LTexture gLevelTextTexture;
LTexture gYourScoreTextTexture;
LTexture gListTextTextures[ 2 * TOTAL_SCORES ];
LTexture gNewScoreTextTextures[ 2 ];
