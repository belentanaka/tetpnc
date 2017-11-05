#ifndef TEXTURES_H
#define TEXTURES_H

#include <SDL2/SDL.h>

#include "../constants.h"
#include "../LTexture/LTexture.h"

// Menu/Gameplay textures
extern LTexture gBlankBGTexture;
extern LTexture gPressEnterTexture;
extern LTexture gPlayBGTexture;
extern LTexture gPausedTexture;
extern LTexture gGameOverTexture;
extern LTexture gEnterNameTexture;
extern LTexture gBlackTexture;
extern LTexture gTetrisTexture;

// Background textures
extern LTexture gBGTextures[ TOTAL_BG ];

// Hand textures and animation clips
extern LTexture gHandBlackTexture;
extern LTexture gHandWhiteTexture;
extern SDL_Rect gHandClips[ 5 ];

// High score textures and clips
extern LTexture gListTexture;
extern SDL_Rect gListClips[ TOTAL_SCORES ];

// Square sprites and clips 
extern LTexture gSquareSpriteTexture;
extern SDL_Rect gSquareSpriteClips[ SQUARE_SPRITE_TOTAL ];

// Text textures
extern LTexture gScoreTextTexture;
extern LTexture gLinesTextTexture;
extern LTexture gLevelTextTexture;
extern LTexture gYourScoreTextTexture;
extern LTexture gListTextTextures[ 2 * TOTAL_SCORES ];
extern LTexture gNewScoreTextTextures[ 2 ];

#endif
