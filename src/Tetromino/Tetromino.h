#ifndef TETROMINO_H
#define TETROMINO_H

#include <SDL2/SDL.h>

#include "../Square/Square.h"
#include "../Timer/Timer.h"

class Tetromino
{
  public:
  Tetromino( TetrominoFlag type, Square gridSquares[], Uint32 fallDelay );
  ~Tetromino();

  bool fall();
  void handleEvent( SDL_Event& e );

  TetrominoFlag getType();

  private:
  void clear();
  void draw();
  void erase();
  void land();

  Timer mTimer;
  Square* mGridSquares;
  TetrominoFlag mType;
  int mGridPositions[ 4 ];
  int mGhostPositions[ 4 ];
  int mRotation;
  Uint32 mFallDelay;
  Uint32 mInitialFallDelay;
};

#endif
