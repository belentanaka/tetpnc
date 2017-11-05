#ifndef PLAY_H
#define PLAY_H

#include <SDL2/SDL.h>

#include "../globals/globals.h"
#include "../Square/Square.h"
#include "../Tetromino/Tetromino.h"
#include "../GameState/GameState.h"

class Play : public GameState
{
  public:
  Play( Stats* stats, Square* gridSquares, Square** nextSquares, Square** holdSquares, SDL_Rect* statAreas );
  ~Play();

  void handleEvent( SDL_Event& e );
  void logic();
  void render();

  private:
  bool createTetromino( TetrominoFlag type );
  void updateNext();
  void updateHold();

  Stats* mStats;
  Square* mGridSquares;
  Square** mNextSquares;
  Square** mHoldSquares;
  Tetromino* mTetromino;
  SDL_Point mStatCenters[ 3 ];
  SDL_Point mStatPositions[ 3 ];
  bool mStarted;
  bool mPaused;
  bool mHolding;
  bool mStatsChanged;
  bool mClearing;
  bool mTetris;
};

#endif
