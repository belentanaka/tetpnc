#ifndef SQUARE_H
#define SQUARE_H

#include <SDL2/SDL.h>

// Used to render Tetromino blocks
class Square
{
  public:
  static const int SQUARE_WIDTH = 25;
  static const int SQUARE_HEIGHT = 25;

  Square();

  void setPosition( int x, int y );
  void fill( int type );
  void stop();
  void clear();
  void replace( Square& s );
  void setAlpha( Uint8 alpha );
  void render();

  int getState();

  private:
  SDL_Point mPosition;
  SquareState mCurrentState;
  SquareSprite mCurrentSprite;
  Uint8 mAlpha;
};

#endif
