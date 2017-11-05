#ifndef INTRO_H
#define INTRO_H

#include <SDL2/SDL.h>

#include "../constants.h"
#include "../Square/Square.h"
#include "../GameState/GameState.h"

class Intro : public GameState
{
  public:
  Intro( Square** introSquares, SDL_Rect& listArea );
  ~Intro();

  void handleEvent( SDL_Event& e );
  void logic();
  void render();

  private:
  Square** mIntroSquares;
  SDL_Point mListCenters[ TOTAL_SCORES ];
  SDL_Point mListPositions[ TOTAL_SCORES ];
};

#endif
