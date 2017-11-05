#ifndef GAMEOVER_H
#define GAMEOVER_H

#include <SDL2/SDL.h>

#include "../constants.h"
#include "../Square/Square.h"
#include "../GameState/GameState.h"

class GameOver : public GameState
{
  public:
  GameOver( Stats* stats, Square* gridSquares, SDL_Rect& yourScoreArea );
  ~GameOver();

  void handleEvent( SDL_Event& e );
  void logic();
  void render();

  private:
  Square* mGridSquares;
  SDL_Point mYourScorePosition;
  int mLastBG;
  int mSquareSequence[ TOTAL_SQUARES ];
};

#endif
