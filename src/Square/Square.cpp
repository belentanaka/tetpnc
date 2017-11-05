#include <SDL2/SDL.h>

#include "../constants.h"
#include "../LTexture/LTexture.h"
#include "../textures/textures.h"
#include "Square.h"

// Initialize variables
Square::Square()
{
  mPosition.x = 0;
  mPosition.y = 0;

  mCurrentState = SQUARE_STATE_BLANK;
  mCurrentSprite = SQUARE_SPRITE_BLANK;
  mAlpha = 255;
}

// Set (x,y) location in window
void Square::setPosition( int x, int y )
{
  mPosition.x = x;
  mPosition.y = y;
}

// Draw Tetromino block in Square and adjust state
void Square::fill( int type )
{
  switch( type )
  {
    case TETROMINO_I:
      mCurrentState = SQUARE_STATE_MOVING;
      mCurrentSprite = SQUARE_SPRITE_I;
      mAlpha = 255;
      break;
    
    case TETROMINO_J:
      mCurrentState = SQUARE_STATE_MOVING;
      mCurrentSprite = SQUARE_SPRITE_J;
      mAlpha = 255;
      break;
    
    case TETROMINO_L:
      mCurrentState = SQUARE_STATE_MOVING;
      mCurrentSprite = SQUARE_SPRITE_L;
      mAlpha = 255;
      break;
    
    case TETROMINO_O:
      mCurrentState = SQUARE_STATE_MOVING;
      mCurrentSprite = SQUARE_SPRITE_O;
      mAlpha = 255;
      break;
    
    case TETROMINO_S:
      mCurrentState = SQUARE_STATE_MOVING;
      mCurrentSprite = SQUARE_SPRITE_S;
      mAlpha = 255;
      break;
      
    case TETROMINO_T:
      mCurrentState = SQUARE_STATE_MOVING;
      mCurrentSprite = SQUARE_SPRITE_T;
      mAlpha = 255;
      break;

    case TETROMINO_Z:
      mCurrentState = SQUARE_STATE_MOVING;
      mCurrentSprite = SQUARE_SPRITE_Z;
      mAlpha = 255;
      break;
  }
}

// Stop Tetromino block from falling
void Square::stop()
{
  mCurrentState = SQUARE_STATE_STILL;
}

// Clear Tetromino block from Square and adjust state
void Square::clear()
{
  mCurrentState = SQUARE_STATE_BLANK;
  mCurrentSprite = SQUARE_SPRITE_BLANK;
  mAlpha = 255;
}

// Replace Square state and sprite with that of another
void Square::replace( Square& s )
{
  mCurrentState = s.mCurrentState;
  mCurrentSprite = s.mCurrentSprite;
} 

// Adjust alpha
void Square::setAlpha( Uint8 alpha )
{
  mAlpha = alpha;
}

// Render image at Square
void Square::render()
{
  gSquareSpriteTexture.setAlpha( mAlpha );
  gSquareSpriteTexture.render( mPosition.x, mPosition.y, &gSquareSpriteClips[ mCurrentSprite ] );
}

// Access state
int Square::getState()
{
  return mCurrentState;
}
