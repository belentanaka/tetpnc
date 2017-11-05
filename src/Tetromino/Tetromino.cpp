#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>

#include "../constants.h"
#include "../globals/globals.h"
#include "../Square/Square.h"
#include "Tetromino.h"

// Initialize member variables
Tetromino::Tetromino( TetrominoFlag type, Square gridSquares[], Uint32 fallDelay )
{
  mRotation = 0;

  mTimer.start();

  mFallDelay = fallDelay;
  mInitialFallDelay = fallDelay;

  mGridSquares = gridSquares;
  if( mGridSquares == NULL )
  {
    clear();
  }
  else
  {
    mType = type;

    int start = ( TOTAL_COLS * 3 / 2 ) - 2;
    int a, b, c, d;

    // Pick which Squares to fill in based on type
    switch( mType )
    {
      case TETROMINO_I:
	a = start + TOTAL_COLS;
	b = start + TOTAL_COLS + 1;
	c = start + TOTAL_COLS + 2;
	d = start + TOTAL_COLS + 3;
	break;
      
      case TETROMINO_J:
	a = start;
	b = start + TOTAL_COLS;
	c = start + TOTAL_COLS + 1;
	d = start + TOTAL_COLS + 2;
	break;

      case TETROMINO_L:
	a = start + 2;
	b = start + TOTAL_COLS;
	c = start + TOTAL_COLS + 1;
	d = start + TOTAL_COLS + 2;
	break;

      case TETROMINO_O:
	a = start + 1;
	b = start + 2;
	c = start + TOTAL_COLS + 1;
	d = start + TOTAL_COLS + 2;
	break;
      
      case TETROMINO_S:
	a = start + 1;
	b = start + 2;
	c = start + TOTAL_COLS;
	d = start + TOTAL_COLS + 1;
	break;

      case TETROMINO_T:
	a = start + 1;
	b = start + TOTAL_COLS;
	c = start + TOTAL_COLS + 1;
	d = start + TOTAL_COLS + 2;
	break;

      case TETROMINO_Z:
	a = start;
	b = start + 1;
	c = start + TOTAL_COLS + 1;
	d = start + TOTAL_COLS + 2;
	break;

      default:
	clear();
	return;
    }

    // Check if lower starting location is already filled
    if( mGridSquares[ a ].getState() == SQUARE_STATE_STILL ||
	mGridSquares[ b ].getState() == SQUARE_STATE_STILL ||
	mGridSquares[ c ].getState() == SQUARE_STATE_STILL ||
	mGridSquares[ d ].getState() == SQUARE_STATE_STILL )
    {
      a -= TOTAL_COLS;
      b -= TOTAL_COLS;
      c -= TOTAL_COLS;
      d -= TOTAL_COLS;

      // Check if higher starting location is already filled
      if( mGridSquares[ a ].getState() == SQUARE_STATE_STILL ||
	  mGridSquares[ b ].getState() == SQUARE_STATE_STILL ||
	  mGridSquares[ c ].getState() == SQUARE_STATE_STILL ||
	  mGridSquares[ d ].getState() == SQUARE_STATE_STILL )
      {
	clear();
      }
      else
      {
	mGridPositions[ 0 ] = a;
	mGridPositions[ 1 ] = b;
	mGridPositions[ 2 ] = c;
	mGridPositions[ 3 ] = d;

	draw();
      }
    }
    else
    {
      mGridPositions[ 0 ] = a;
      mGridPositions[ 1 ] = b;
      mGridPositions[ 2 ] = c;
      mGridPositions[ 3 ] = d;

      draw();
    }
  }
}

// Clear out the Tetromino blocks
Tetromino::~Tetromino()
{
  clear();
}

// Deals with falling logic
bool Tetromino::fall()
{
  bool falling = true;

  int currentTime = mTimer.getTicks();

  if( currentTime >= mFallDelay )
  {
    int a, b, c, d;
    a = mGridPositions[ 0 ] + TOTAL_COLS;
    b = mGridPositions[ 1 ] + TOTAL_COLS;
    c = mGridPositions[ 2 ] + TOTAL_COLS;
    d = mGridPositions[ 3 ] + TOTAL_COLS;

    // Check if the Tetromino has hit the bottom or another block
    if( a >= TOTAL_SQUARES ||
	b >= TOTAL_SQUARES ||
	c >= TOTAL_SQUARES ||
	d >= TOTAL_SQUARES ||
	mGridSquares[ a ].getState() == SQUARE_STATE_STILL ||
	mGridSquares[ b ].getState() == SQUARE_STATE_STILL ||
	mGridSquares[ c ].getState() == SQUARE_STATE_STILL ||
	mGridSquares[ d ].getState() == SQUARE_STATE_STILL )
    {
      falling = false;
      land();
    }
    else
    {
      erase();

      mGridPositions[ 0 ] = a;
      mGridPositions[ 1 ] = b;
      mGridPositions[ 2 ] = c;
      mGridPositions[ 3 ] = d;

      draw();
    }

    mTimer.start();
  }
   
  return falling;
}

// Handle key presses
void Tetromino::handleEvent( SDL_Event& e )
{
  if( e.type == SDL_KEYDOWN )
  {
    // Move Tetromino to the left
    if( e.key.keysym.sym == SDLK_LEFT )
    {
      int a, b, c, d;
      a = mGridPositions[ 0 ] - 1;
      b = mGridPositions[ 1 ] - 1;
      c = mGridPositions[ 2 ] - 1;
      d = mGridPositions[ 3 ] - 1;
      
      // Check if Tetromino will be in bounds and will not overlap other blocks
      if( mGridPositions[ 0 ] % TOTAL_COLS > 0 &&
	  mGridPositions[ 1 ] % TOTAL_COLS > 0 &&
	  mGridPositions[ 2 ] % TOTAL_COLS > 0 &&
	  mGridPositions[ 3 ] % TOTAL_COLS > 0 &&
	  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
      {
	erase();

	mGridPositions[ 0 ] = a;
	mGridPositions[ 1 ] = b;
	mGridPositions[ 2 ] = c;
	mGridPositions[ 3 ] = d;

	draw();

	Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );
      }
    }

    // Move Tetromino to the right
    if( e.key.keysym.sym == SDLK_RIGHT )
    {
      int a, b, c, d;
      a = mGridPositions[ 0 ] + 1;
      b = mGridPositions[ 1 ] + 1;
      c = mGridPositions[ 2 ] + 1;
      d = mGridPositions[ 3 ] + 1;

      // Check if Tetromino will be in bounds and will not overlap other blocks
      if( mGridPositions[ 0 ] % TOTAL_COLS < TOTAL_COLS - 1 &&
	  mGridPositions[ 1 ] % TOTAL_COLS < TOTAL_COLS - 1 &&
	  mGridPositions[ 2 ] % TOTAL_COLS < TOTAL_COLS - 1 &&
	  mGridPositions[ 3 ] % TOTAL_COLS < TOTAL_COLS - 1 &&
	  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
      {
	erase();

	mGridPositions[ 0 ] = a;
	mGridPositions[ 1 ] = b;
	mGridPositions[ 2 ] = c;
	mGridPositions[ 3 ] = d;
	
	draw();

	Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );
      }
    }

    // Rotate Tetromino clockwise
    if( e.key.keysym.sym == SDLK_x || e.key.keysym.sym == SDLK_UP )
    {
      int a, b, c, d;

      if( mType == TETROMINO_I )
      {
	if( mRotation == 0 )
	{
	  // Attempt rotation
	  b = mGridPositions[ 2 ];
	  a = b - TOTAL_COLS;
	  c = b + TOTAL_COLS;
	  d = c + TOTAL_COLS;

	  // Check if rotated position will be valid
	  if( b / TOTAL_COLS > 0 &&
	      b / TOTAL_COLS < TOTAL_ROWS - 2 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL && 
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    erase();
	    
	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 1;

	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );
	  }
	  else
	  {
	    // Attempt wall kick
	    b = mGridPositions[ 2 ] - TOTAL_COLS;
	    a = b - TOTAL_COLS;
	    c = b + TOTAL_COLS;
	    d = c + TOTAL_COLS;

	    // Check if rotated position will be valid
	    if( b / TOTAL_COLS > 0 &&
		b / TOTAL_COLS < TOTAL_ROWS - 2 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL && 
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {	      
	      erase();
	     
	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 1;

	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );
	    }
	    else
	    {
	      // Attempt wall kick
	      b = mGridPositions[ 2 ] - TOTAL_COLS - TOTAL_COLS;
	      a = b - TOTAL_COLS;
	      c = b + TOTAL_COLS;
	      d = c + TOTAL_COLS;

	      // Check if rotated position will be valid
	      if( b / TOTAL_COLS > 0 &&
		  b / TOTAL_COLS < TOTAL_ROWS - 2 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL && 
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;
		
		draw();

		mRotation = 1;

		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );
	      }
	      else
	      {
		// Attempt wall kick
		b = mGridPositions[ 2 ] + TOTAL_COLS;
		a = b - TOTAL_COLS;
		c = b + TOTAL_COLS;
		d = c + TOTAL_COLS;

		// Check if rotated position will be valid
		if( b / TOTAL_COLS > 0 &&
		    b / TOTAL_COLS < TOTAL_ROWS - 2 &&
		    mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		    mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		    mGridSquares[ c ].getState() != SQUARE_STATE_STILL && 
		    mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
		{
		  erase();

		  mGridPositions[ 0 ] = a;
		  mGridPositions[ 1 ] = b;
		  mGridPositions[ 2 ] = c;
		  mGridPositions[ 3 ] = d;

		  draw();

		  mRotation = 1;

		  Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );
		}
	      }
	    }
	  }
	}
	else if( mRotation == 1 )
	{
	  c = mGridPositions[ 2 ];
	  b = c - 1;
	  a = b - 1;
	  d = c + 1;

	  if( c % TOTAL_COLS > 1 &&
	      c % TOTAL_COLS < TOTAL_COLS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 2;

	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );
	  }
	  else
	  {
	    c = mGridPositions[ 2 ] - 1;
	    b = c - 1;
	    a = b - 1;
	    d = c + 1;

	    if( c % TOTAL_COLS > 1 &&
		c % TOTAL_COLS < TOTAL_COLS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 2;

	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );
	    }
	    else
	    {
	      c = mGridPositions[ 2 ] + 1;
	      b = c - 1;
	      a = b - 1;
	      d = c + 1;

	      if( c % TOTAL_COLS > 1 &&
		  c % TOTAL_COLS < TOTAL_COLS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 2;

		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );
	      }
	      else
	      {
		c = mGridPositions[ 2 ] + 2;
		b = c - 1;
		a = b - 1;
		d = c + 1;

		if( c % TOTAL_COLS > 1 &&
		    c % TOTAL_COLS < TOTAL_COLS - 1 &&
		    mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		    mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		    mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		    mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
		{
		  erase();

		  mGridPositions[ 0 ] = a;
		  mGridPositions[ 1 ] = b;
		  mGridPositions[ 2 ] = c;
		  mGridPositions[ 3 ] = d;

		  draw();

		  mRotation = 2;

		  Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );
		}
	      }
	    }
	  }
	}
	else if( mRotation == 2 )
	{
	  c = mGridPositions[ 1 ];
	  b = c - TOTAL_COLS;
	  a = b - TOTAL_COLS;
	  d = c + TOTAL_COLS;

	  if( c / TOTAL_COLS > 1 &&
	      c / TOTAL_COLS < TOTAL_ROWS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 3;
	  }
	  else
	  {
	    c = mGridPositions[ 1 ] - TOTAL_COLS;
	    b = c - TOTAL_COLS;
	    a = b - TOTAL_COLS;
	    d = c + TOTAL_COLS;

	    if( c / TOTAL_COLS > 1 &&
		c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 3;
	    }
	    else
	    {
	      c = mGridPositions[ 1 ] + TOTAL_COLS;
	      b = c - TOTAL_COLS;
	      a = b - TOTAL_COLS;
	      d = c + TOTAL_COLS;

	      if( c / TOTAL_COLS > 1 &&
		  c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 3;
	      }
	      else
	      {
		c = mGridPositions[ 1 ] + TOTAL_COLS + TOTAL_COLS;
		b = c - TOTAL_COLS;
		a = b - TOTAL_COLS;
		d = c + TOTAL_COLS;

		if( c / TOTAL_COLS > 1 &&
		    c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		    mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		    mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		    mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		    mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
		{
		  Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		  erase();

		  mGridPositions[ 0 ] = a;
		  mGridPositions[ 1 ] = b;
		  mGridPositions[ 2 ] = c;
		  mGridPositions[ 3 ] = d;

		  draw();

		  mRotation = 3;
		}
	      }
	    }
	  }
	}
	else if( mRotation == 3 )
	{
	  b = mGridPositions[ 1 ];
	  a = b - 1;
	  c = b + 1;
	  d = c + 1;

	  if( b % TOTAL_COLS > 0 &&
	      b % TOTAL_COLS < TOTAL_COLS - 2 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 0;
	  }
	  else
	  {
	    b = mGridPositions[ 1 ] - 1;
	    a = b - 1;
	    c = b + 1;
	    d = c + 1;

	    if( b % TOTAL_COLS > 0 &&
		b % TOTAL_COLS < TOTAL_COLS - 2 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 0;
	    }
	    else
	    {
	      b = mGridPositions[ 1 ] - 2;
	      a = b - 1;
	      c = b + 1;
	      d = c + 1;
	      
	      if( b % TOTAL_COLS > 0 &&
		  b % TOTAL_COLS < TOTAL_COLS - 2 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 0;
	      }
	      else
	      {
		b = mGridPositions[ 1 ] + 1;
		a = b - 1;
		c = b + 1;
		d = c + 1;
		
		if( b % TOTAL_COLS > 0 &&
		    b % TOTAL_COLS < TOTAL_COLS - 2 &&
		    mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		    mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		    mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		    mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
		{
		  Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		  erase();

		  mGridPositions[ 0 ] = a;
		  mGridPositions[ 1 ] = b;
		  mGridPositions[ 2 ] = c;
		  mGridPositions[ 3 ] = d;

		  draw();

		  mRotation = 0;
		}
	      }
	    }
	  }
	}
      }
      if( mType == TETROMINO_J )
      {
	if( mRotation == 0 )
	{
	  c = mGridPositions[ 2 ];
	  a = c - TOTAL_COLS;
	  b = a + 1;
	  d = c + TOTAL_COLS;

	  if( c / TOTAL_COLS > 0 &&
	      c / TOTAL_COLS < TOTAL_ROWS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();
	    
	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 1;
	  }
	  else
	  {
	    c = mGridPositions[ 2 ] - TOTAL_COLS;
	    a = c - TOTAL_COLS;
	    b = a + 1;
	    d = c + TOTAL_COLS;

	    if( c / TOTAL_COLS > 0 &&
		c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();
	      
	      mRotation = 1;
	    }
	    else
	    {
	      c = mGridPositions[ 2 ] + TOTAL_COLS;
	      a = c - TOTAL_COLS;
	      b = a + 1;
	      d = c + TOTAL_COLS;

	      if( c / TOTAL_COLS > 0 &&
		  c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 1;
	      }
	    }
	  }
	}
	else if( mRotation == 1 )
	{
	  b = mGridPositions[ 2 ];
	  a = b - 1;
	  c = b + 1;
	  d = c + TOTAL_COLS;

	  if( b % TOTAL_COLS > 0 &&
	      b % TOTAL_COLS < TOTAL_COLS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 2;
	  }
	  else
	  {
	    b = mGridPositions[ 2 ] - 1;
	    a = b - 1;
	    c = b + 1;
	    d = c + TOTAL_COLS;

	    if( b % TOTAL_COLS > 0 &&
		b % TOTAL_COLS < TOTAL_COLS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();
	      
	      mRotation = 2;
	    }
	    else
	    {
	      b = mGridPositions[ 2 ] + 1;
	      a = b - 1;
	      c = b + 1;
	      d = c + TOTAL_COLS;

	      if( b % TOTAL_COLS > 0 &&
		  b % TOTAL_COLS < TOTAL_COLS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();
	      
		mRotation = 2;
	      }
	    }
	  }
	}
	else if( mRotation == 2 )
	{
	  b = mGridPositions[ 1 ];
	  a = b - TOTAL_COLS;
	  d = b + TOTAL_COLS;
	  c = d - 1;

	  if( b / TOTAL_COLS > 0 &&
	      b / TOTAL_COLS < TOTAL_ROWS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;
	    
	    draw();
	    
	    mRotation = 3;
	  }
	  else
	  {
	    b = mGridPositions[ 1 ] - TOTAL_COLS;
	    a = b - TOTAL_COLS;
	    d = b + TOTAL_COLS;
	    c = d - 1;
	    
	    if( b / TOTAL_COLS > 0 &&
		b / TOTAL_COLS < TOTAL_ROWS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 3;
	    }
	    else
	    {
	      b = mGridPositions[ 1 ] + TOTAL_COLS;
	      a = b - TOTAL_COLS;
	      d = b + TOTAL_COLS;
	      c = d - 1;

	      if( b / TOTAL_COLS > 0 &&
		  b / TOTAL_COLS < TOTAL_ROWS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 3;
	      }
	    }
	  }
	}
	else if( mRotation == 3 )
	{
	  c = mGridPositions[ 1 ];
	  b = c - 1;
	  a = b - TOTAL_COLS;
	  d = c + 1;

	  if( c % TOTAL_COLS > 0 &&
	      c % TOTAL_COLS < TOTAL_COLS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );
	   
	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 0;
	  }
	  else
	  {
	    c = mGridPositions[ 1 ] - 1;
	    b = c - 1;
	    a = b - TOTAL_COLS;
	    d = c + 1;

	    if( c % TOTAL_COLS > 0 &&
		c % TOTAL_COLS < TOTAL_COLS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 0;
	    }
	    else
	    {
	      c = mGridPositions[ 1 ] + 1;
	      b = c - 1;
	      a = b - TOTAL_COLS;
	      d = c + 1;

	      if( c % TOTAL_COLS > 0 &&
		  c % TOTAL_COLS < TOTAL_COLS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 0;
	      }
	    }
	  }
	}
      }
      if( mType == TETROMINO_L )
      {
	if( mRotation == 0 )
	{
	  b = mGridPositions[ 2 ];
	  a = b - TOTAL_COLS;
	  c = b + TOTAL_COLS;
	  d = c + 1;

	  if( b / TOTAL_COLS > 0 &&
	      b / TOTAL_COLS < TOTAL_ROWS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );
	    
	    erase();
	    
	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 1;
	  }
	  else
	  {
	    b = mGridPositions[ 2 ] - TOTAL_COLS;
	    a = b - TOTAL_COLS;
	    c = b + TOTAL_COLS;
	    d = c + 1;

	    if( b / TOTAL_COLS > 0 &&
		b / TOTAL_COLS < TOTAL_ROWS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();
	      
	      mRotation = 1;
	    }
	    else
	    {
	      b = mGridPositions[ 2 ] + TOTAL_COLS;
	      a = b - TOTAL_COLS;
	      c = b + TOTAL_COLS;
	      d = c + 1;

	      if( b / TOTAL_COLS > 0 &&
		  b / TOTAL_COLS < TOTAL_ROWS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 1;
	      }
	    }
	  }
	}
	else if( mRotation == 1 )
	{
	  b = mGridPositions[ 1 ];
	  a = b - 1;
	  d = a + TOTAL_COLS;
	  c = b + 1;

	  if( b % TOTAL_COLS > 0 &&
	      b % TOTAL_COLS < TOTAL_COLS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 2;
	  }
	  else
	  {
	    b = mGridPositions[ 1 ] - 1;
	    a = b - 1;
	    d = a + TOTAL_COLS;
	    c = b + 1;

	    if( b % TOTAL_COLS > 0 &&
		b % TOTAL_COLS < TOTAL_COLS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();
	      
	      mRotation = 2;
	    }
	    else
	    {
	      b = mGridPositions[ 1 ] + 1;
	      a = b - 1;
	      d = a + TOTAL_COLS;
	      c = b + 1;

	      if( b % TOTAL_COLS > 0 &&
		  b % TOTAL_COLS < TOTAL_COLS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();
	      
		mRotation = 2;
	      }
	    }
	  }
	}
	else if( mRotation == 2 )
	{
	  c = mGridPositions[ 1 ];
	  b = c - TOTAL_COLS;
	  a = b - 1;
	  d = c + TOTAL_COLS;

	  if( c / TOTAL_COLS > 0 &&
	      c / TOTAL_COLS < TOTAL_ROWS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;
	    
	    draw();
	    
	    mRotation = 3;
	  }
	  else
	  {
	    c = mGridPositions[ 1 ] - TOTAL_COLS;
	    b = c - TOTAL_COLS;
	    a = b - 1;
	    d = c + TOTAL_COLS;
	    
	    if( c / TOTAL_COLS > 0 &&
		c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 3;
	    }
	    else
	    {
	      c = mGridPositions[ 1 ] + TOTAL_COLS;
	      b = c - TOTAL_COLS;
	      a = b - 1;
	      d = c + TOTAL_COLS;

	      if( c / TOTAL_COLS > 0 &&
		  c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 3;
	      }
	    }
	  }
	}
	else if( mRotation == 3 )
	{
	  c = mGridPositions[ 2 ];
	  b = c - 1;
	  d = c + 1;
	  a = d - TOTAL_COLS;

	  if( c % TOTAL_COLS > 0 &&
	      c % TOTAL_COLS < TOTAL_COLS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 0;
	  }
	  else
	  {
	    c = mGridPositions[ 2 ] - 1;
	    b = c - 1;
	    d = c + 1;
	    a = d - TOTAL_COLS;

	    if( c % TOTAL_COLS > 0 &&
		c % TOTAL_COLS < TOTAL_COLS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 0;
	    }
	    else
	    {
	      c = mGridPositions[ 2 ] + 1;
	      b = c - 1;
	      d = c + 1;
	      a = d - TOTAL_COLS;

	      if( c % TOTAL_COLS > 0 &&
		  c % TOTAL_COLS < TOTAL_COLS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 0;
	      }
	    }
	  }
	}
      }
      if( mType == TETROMINO_O )
      {
	Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );
      }
      if( mType == TETROMINO_S )
      {
	if( mRotation == 0 )
	{
	  b = mGridPositions[ 3 ];
	  a = b - TOTAL_COLS;
	  c = b + 1;
	  d = c + TOTAL_COLS;

	  if( b / TOTAL_COLS > 0 &&
	      b / TOTAL_COLS < TOTAL_ROWS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();
	    
	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 1;
	  }
	  else
	  {
	    b = mGridPositions[ 3 ] - TOTAL_COLS;
	    a = b - TOTAL_COLS;
	    c = b + 1;
	    d = c + TOTAL_COLS;

	    if( b / TOTAL_COLS > 0 &&
		b / TOTAL_COLS < TOTAL_ROWS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();
	      
	      mRotation = 1;
	    }
	    else
	    {
	      b = mGridPositions[ 3 ] + TOTAL_COLS;
	      a = b - TOTAL_COLS;
	      c = b + 1;
	      d = c + TOTAL_COLS;

	      if( b / TOTAL_COLS > 0 &&
		  b / TOTAL_COLS < TOTAL_ROWS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 1;
	      }
	    }
	  }
	}
	else if( mRotation == 1 )
	{
	  a = mGridPositions[ 1 ];
	  b = a + 1;
	  d = a + TOTAL_COLS;
	  c = d - 1;

	  if( a % TOTAL_COLS > 0 &&
	      a % TOTAL_COLS < TOTAL_COLS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 2;
	  }
	  else
	  {
	    a = mGridPositions[ 1 ] - 1;
	    b = a + 1;
	    d = a + TOTAL_COLS;
	    c = d - 1;

	    if( a % TOTAL_COLS > 0 &&
		a % TOTAL_COLS < TOTAL_COLS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();
	      
	      mRotation = 2;
	    }
	    else
	    {
	      a = mGridPositions[ 1 ] + 1;
	      b = a + 1;
	      d = a + TOTAL_COLS;
	      c = d - 1;

	      if( a % TOTAL_COLS > 0 &&
		  a % TOTAL_COLS < TOTAL_COLS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();
	      
		mRotation = 2;
	      }
	    }
	  }
	}
	else if( mRotation == 2 )
	{
	  c = mGridPositions[ 0 ];
	  b = c - 1;
	  a = b - TOTAL_COLS;
	  d = c + TOTAL_COLS;

	  if( c / TOTAL_COLS > 0 &&
	      c / TOTAL_COLS < TOTAL_ROWS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;
	    
	    draw();
	    
	    mRotation = 3;
	  }
	  else
	  {
	    c = mGridPositions[ 0 ] - TOTAL_COLS;
	    b = c - 1;
	    a = b - TOTAL_COLS;
	    d = c + TOTAL_COLS;
	    
	    if( c / TOTAL_COLS > 0 &&
		c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 3;
	    }
	    else
	    {
	      c = mGridPositions[ 0 ] + TOTAL_COLS;
	      b = c - 1;
	      a = b - TOTAL_COLS;
	      d = c + TOTAL_COLS;

	      if( c / TOTAL_COLS > 0 &&
		  c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 3;
	      }
	    }
	  }
	}
	else if( mRotation == 3 )
	{
	  d = mGridPositions[ 2 ];
	  a = d - TOTAL_COLS;
	  b = a + 1;
	  c = d - 1;

	  if( d % TOTAL_COLS > 0 &&
	      d % TOTAL_COLS < TOTAL_COLS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 0;
	  }
	  else
	  {
	    d = mGridPositions[ 2 ] - 1;
	    a = d - TOTAL_COLS;
	    b = a + 1;
	    c = d - 1;

	    if( d % TOTAL_COLS > 0 &&
		d % TOTAL_COLS < TOTAL_COLS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 0;
	    }
	    else
	    {
	      d = mGridPositions[ 2 ] + 1;
	      a = d - TOTAL_COLS;
	      b = a + 1;
	      c = d - 1;

	      if( d % TOTAL_COLS > 0 &&
		  d % TOTAL_COLS < TOTAL_COLS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 0;
	      }
	    }
	  }
	}
      }
      if( mType == TETROMINO_T )
      {
	if( mRotation == 0 )
	{
	  b = mGridPositions[ 2 ];
	  a = b - TOTAL_COLS;
	  c = b + 1;
	  d = b + TOTAL_COLS;

	  if( b / TOTAL_COLS > 0 &&
	      b / TOTAL_COLS < TOTAL_ROWS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();
	    
	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 1;
	  }
	  else
	  {
	    b = mGridPositions[ 2 ] - TOTAL_COLS;
	    a = b - TOTAL_COLS;
	    c = b + 1;
	    d = b + TOTAL_COLS;

	    if( b / TOTAL_COLS > 0 &&
		b / TOTAL_COLS < TOTAL_ROWS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();
	      
	      mRotation = 1;
	    }
	    else
	    {
	      b = mGridPositions[ 2 ] + TOTAL_COLS;
	      a = b - TOTAL_COLS;
	      c = b + 1;
	      d = b + TOTAL_COLS;

	      if( b / TOTAL_COLS > 0 &&
		  b / TOTAL_COLS < TOTAL_ROWS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 1;
	      }
	    }
	  }
	}
	else if( mRotation == 1 )
	{
	  b = mGridPositions[ 1 ];
	  a = b - 1;
	  c = b + 1;
	  d = b + TOTAL_COLS;

	  if( b % TOTAL_COLS > 0 &&
	      b % TOTAL_COLS < TOTAL_COLS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 2;
	  }
	  else
	  {
	    b = mGridPositions[ 1 ] - 1;
	    a = b - 1;
	    c = b + 1;
	    d = b + TOTAL_COLS;

	    if( b % TOTAL_COLS > 0 &&
		b % TOTAL_COLS < TOTAL_COLS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();
	      
	      mRotation = 2;
	    }
	    else
	    {
	      b = mGridPositions[ 1 ] + 1;
	      a = b - 1;
	      c = b + 1;
	      d = b + TOTAL_COLS;

	      if( b % TOTAL_COLS > 0 &&
		  b % TOTAL_COLS < TOTAL_COLS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();
	      
		mRotation = 2;
	      }
	    }
	  }
	}
	else if( mRotation == 2 )
	{
	  c = mGridPositions[ 1 ];
	  b = c - 1;
	  a = c - TOTAL_COLS;
	  d = c + TOTAL_COLS;

	  if( c / TOTAL_COLS > 0 &&
	      c / TOTAL_COLS < TOTAL_ROWS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;
	    
	    draw();
	    
	    mRotation = 3;
	  }
	  else
	  {
	    c = mGridPositions[ 1 ] - TOTAL_COLS;
	    b = c - 1;
	    a = c - TOTAL_COLS;
	    d = c + TOTAL_COLS;
	    
	    if( c / TOTAL_COLS > 0 &&
		c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 3;
	    }
	    else
	    {
	      c = mGridPositions[ 1 ] + TOTAL_COLS;
	      b = c - 1;
	      a = c - TOTAL_COLS;
	      d = c + TOTAL_COLS;

	      if( c / TOTAL_COLS > 0 &&
		  c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 3;
	      }
	    }
	  }
	}
	else if( mRotation == 3 )
	{
	  c = mGridPositions[ 2 ];
	  a = c - TOTAL_COLS;
	  b = c - 1;
	  d = c + 1;

	  if( c % TOTAL_COLS > 0 &&
	      c % TOTAL_COLS < TOTAL_COLS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 0;
	  }
	  else
	  {
	    c = mGridPositions[ 2 ] - 1;
	    a = c - TOTAL_COLS;
	    b = c - 1;
	    d = c + 1;

	    if( c % TOTAL_COLS > 0 &&
		c % TOTAL_COLS < TOTAL_COLS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 0;
	    }
	    else
	    {
	      c = mGridPositions[ 2 ] + 1;
	      a = c - TOTAL_COLS;
	      b = c - 1;
	      d = c + 1;

	      if( c % TOTAL_COLS > 0 &&
		  c % TOTAL_COLS < TOTAL_COLS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 0;
	      }
	    }
	  }
	}
      }
      if( mType == TETROMINO_Z )
      {
	if( mRotation == 0 )
	{
	  b = mGridPositions[ 2 ];
	  c = b + 1;
	  a = c - TOTAL_COLS;
	  d = b + TOTAL_COLS;

	  if( b / TOTAL_COLS > 0 &&
	      b / TOTAL_COLS < TOTAL_ROWS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();
	    
	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 1;
	  }
	  else
	  {
	    b = mGridPositions[ 2 ] - TOTAL_COLS;
	    c = b + 1;
	    a = c - TOTAL_COLS;
	    d = b + TOTAL_COLS;

	    if( b / TOTAL_COLS > 0 &&
		b / TOTAL_COLS < TOTAL_ROWS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();
	      
	      mRotation = 1;
	    }
	    else
	    {
	      b = mGridPositions[ 2 ] + TOTAL_COLS;
	      c = b + 1;
	      a = c - TOTAL_COLS;
	      d = b + TOTAL_COLS;

	      if( b / TOTAL_COLS > 0 &&
		  b / TOTAL_COLS < TOTAL_ROWS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 1;
	      }
	    }
	  }
	}
	else if( mRotation == 1 )
	{
	  b = mGridPositions[ 1 ];
	  a = b - 1;
	  c = b + TOTAL_COLS;
	  d = c + 1;

	  if( b % TOTAL_COLS > 0 &&
	      b % TOTAL_COLS < TOTAL_COLS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 2;
	  }
	  else
	  {
	    b = mGridPositions[ 1 ] - 1;
	    a = b - 1;
	    c = b + TOTAL_COLS;
	    d = c + 1;

	    if( b % TOTAL_COLS > 0 &&
		b % TOTAL_COLS < TOTAL_COLS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();
	      
	      mRotation = 2;
	    }
	    else
	    {
	      b = mGridPositions[ 1 ] + 1;
	      a = b - 1;
	      c = b + TOTAL_COLS;
	      d = c + 1;

	      if( b % TOTAL_COLS > 0 &&
		  b % TOTAL_COLS < TOTAL_COLS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();
	      
		mRotation = 2;
	      }
	    }
	  }
	}
	else if( mRotation == 2 )
	{
	  c = mGridPositions[ 1 ];
	  a = c - TOTAL_COLS;
	  b = c - 1;
	  d = b + TOTAL_COLS;

	  if( c / TOTAL_COLS > 0 &&
	      c / TOTAL_COLS < TOTAL_ROWS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;
	    
	    draw();
	    
	    mRotation = 3;
	  }
	  else
	  {
	    c = mGridPositions[ 1 ] - TOTAL_COLS;
	    a = c - TOTAL_COLS;
	    b = c - 1;
	    d = b + TOTAL_COLS;
	    
	    if( c / TOTAL_COLS > 0 &&
		c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 3;
	    }
	    else
	    {
	      c = mGridPositions[ 1 ] + TOTAL_COLS;
	      a = c - TOTAL_COLS;
	      b = c - 1;
	      d = b + TOTAL_COLS;

	      if( c / TOTAL_COLS > 0 &&
		  c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 3;
	      }
	    }
	  }
	}
	else if( mRotation == 3 )
	{
	  c = mGridPositions[ 2 ];
	  b = c - TOTAL_COLS;
	  a = b - 1;
	  d = c + 1;

	  if( c % TOTAL_COLS > 0 &&
	      c % TOTAL_COLS < TOTAL_COLS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );
	
	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 0;
	  }
	  else
	  {
	    c = mGridPositions[ 2 ] - 1;
	    b = c - TOTAL_COLS;
	    a = b - 1;
	    d = c + 1;

	    if( c % TOTAL_COLS > 0 &&
		c % TOTAL_COLS < TOTAL_COLS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 0;
	    }
	    else
	    {
	      c = mGridPositions[ 2 ] + 1;
	      b = c - TOTAL_COLS;
	      a = b - 1;
	      d = c + 1;

	      if( c % TOTAL_COLS > 0 &&
		  c % TOTAL_COLS < TOTAL_COLS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 0;
	      }
	    }
	  }
	}
      }
    }

    // Rotate Tetromino counter clockwise
    if( e.key.keysym.sym == SDLK_z )
    {
      int a, b, c, d;

      if( mType == TETROMINO_I )
      {
	if( mRotation == 0 )
	{
	  b = mGridPositions[ 1 ];
	  a = b - TOTAL_COLS;
	  c = b + TOTAL_COLS;
	  d = c + TOTAL_COLS;

	  if( b / TOTAL_COLS > 0 &&
	      b / TOTAL_COLS < TOTAL_ROWS - 2 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL && 
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();
	    
	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 3;
	  }
	  else
	  {
	    b = mGridPositions[ 1 ] - TOTAL_COLS;
	    a = b - TOTAL_COLS;
	    c = b + TOTAL_COLS;
	    d = c + TOTAL_COLS;

	    if( b / TOTAL_COLS > 0 &&
		b / TOTAL_COLS < TOTAL_ROWS - 2 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL && 
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();
	     
	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 3;
	    }
	    else
	    {
	      b = mGridPositions[ 1 ] - TOTAL_COLS - TOTAL_COLS;
	      a = b - TOTAL_COLS;
	      c = b + TOTAL_COLS;
	      d = c + TOTAL_COLS;

	      if( b / TOTAL_COLS > 0 &&
		  b / TOTAL_COLS < TOTAL_ROWS - 2 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL && 
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;
		
		draw();

		mRotation = 3;
	      }
	      else
	      {
		b = mGridPositions[ 1 ] + TOTAL_COLS;
		a = b - TOTAL_COLS;
		c = b + TOTAL_COLS;
		d = c + TOTAL_COLS;

		if( b / TOTAL_COLS > 0 &&
		    b / TOTAL_COLS < TOTAL_ROWS - 2 &&
		    mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		    mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		    mGridSquares[ c ].getState() != SQUARE_STATE_STILL && 
		    mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
		{
		  Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		  erase();

		  mGridPositions[ 0 ] = a;
		  mGridPositions[ 1 ] = b;
		  mGridPositions[ 2 ] = c;
		  mGridPositions[ 3 ] = d;

		  draw();

		  mRotation = 3;
		}
	      }
	    }
	  }
	}
	else if( mRotation == 1 )
	{
	  c = mGridPositions[ 1 ];
	  b = c - 1;
	  a = b - 1;
	  d = c + 1;

	  if( c % TOTAL_COLS > 1 &&
	      c % TOTAL_COLS < TOTAL_COLS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 0;
	  }
	  else
	  {
	    c = mGridPositions[ 1 ] - 1;
	    b = c - 1;
	    a = b - 1;
	    d = c + 1;

	    if( c % TOTAL_COLS > 1 &&
		c % TOTAL_COLS < TOTAL_COLS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 0;
	    }
	    else
	    {
	      c = mGridPositions[ 1 ] + 1;
	      b = c - 1;
	      a = b - 1;
	      d = c + 1;

	      if( c % TOTAL_COLS > 1 &&
		  c % TOTAL_COLS < TOTAL_COLS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 0;
	      }
	      else
	      {
		c = mGridPositions[ 1 ] + 2;
		b = c - 1;
		a = b - 1;
		d = c + 1;

		if( c % TOTAL_COLS > 1 &&
		    c % TOTAL_COLS < TOTAL_COLS - 1 &&
		    mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		    mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		    mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		    mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
		{
		  Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		  erase();

		  mGridPositions[ 0 ] = a;
		  mGridPositions[ 1 ] = b;
		  mGridPositions[ 2 ] = c;
		  mGridPositions[ 3 ] = d;

		  draw();

		  mRotation = 0;
		}
	      }
	    }
	  }
	}
	else if( mRotation == 2 )
	{
	  c = mGridPositions[ 2 ];
	  b = c - TOTAL_COLS;
	  a = b - TOTAL_COLS;
	  d = c + TOTAL_COLS;

	  if( c / TOTAL_COLS > 1 &&
	      c / TOTAL_COLS < TOTAL_ROWS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 1;
	  }
	  else
	  {
	    c = mGridPositions[ 2 ] - TOTAL_COLS;
	    b = c - TOTAL_COLS;
	    a = b - TOTAL_COLS;
	    d = c + TOTAL_COLS;

	    if( c / TOTAL_COLS > 1 &&
		c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 1;
	    }
	    else
	    {
	      c = mGridPositions[ 2 ] + TOTAL_COLS;
	      b = c - TOTAL_COLS;
	      a = b - TOTAL_COLS;
	      d = c + TOTAL_COLS;

	      if( c / TOTAL_COLS > 1 &&
		  c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 1;
	      }
	      else
	      {
		c = mGridPositions[ 2 ] + TOTAL_COLS + TOTAL_COLS;
		b = c - TOTAL_COLS;
		a = b - TOTAL_COLS;
		d = c + TOTAL_COLS;

		if( c / TOTAL_COLS > 1 &&
		    c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		    mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		    mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		    mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		    mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
		{
		  Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		  erase();

		  mGridPositions[ 0 ] = a;
		  mGridPositions[ 1 ] = b;
		  mGridPositions[ 2 ] = c;
		  mGridPositions[ 3 ] = d;

		  draw();

		  mRotation = 1;
		}
	      }
	    }
	  }
	}
	else if( mRotation == 3 )
	{
	  b = mGridPositions[ 2 ];
	  a = b - 1;
	  c = b + 1;
	  d = c + 1;

	  if( b % TOTAL_COLS > 0 &&
	      b % TOTAL_COLS < TOTAL_COLS - 2 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 2;
	  }
	  else
	  {
	    b = mGridPositions[ 2 ] - 1;
	    a = b - 1;
	    c = b + 1;
	    d = c + 1;

	    if( b % TOTAL_COLS > 0 &&
		b % TOTAL_COLS < TOTAL_COLS - 2 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 2;
	    }
	    else
	    {
	      b = mGridPositions[ 2 ] - 2;
	      a = b - 1;
	      c = b + 1;
	      d = c + 1;
	      
	      if( b % TOTAL_COLS > 0 &&
		  b % TOTAL_COLS < TOTAL_COLS - 2 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 2;
	      }
	      else
	      {
		b = mGridPositions[ 2 ] + 1;
		a = b - 1;
		c = b + 1;
		d = c + 1;
		
		if( b % TOTAL_COLS > 0 &&
		    b % TOTAL_COLS < TOTAL_COLS - 2 &&
		    mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		    mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		    mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		    mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
		{
		  Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		  erase();

		  mGridPositions[ 0 ] = a;
		  mGridPositions[ 1 ] = b;
		  mGridPositions[ 2 ] = c;
		  mGridPositions[ 3 ] = d;

		  draw();

		  mRotation = 2;
		}
	      }
	    }
	  }
	}
      }
      if( mType == TETROMINO_J )
      {
	if( mRotation == 0 )
	{
	  b = mGridPositions[ 2 ];
	  a = b - TOTAL_COLS;
	  d = b + TOTAL_COLS;
	  c = d - 1;

	  if( b / TOTAL_COLS > 0 &&
	      b / TOTAL_COLS < TOTAL_ROWS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();
	    
	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 3;
	  }
	  else
	  {
	    b = mGridPositions[ 2 ] - TOTAL_COLS;
	    a = b - TOTAL_COLS;
	    d = b + TOTAL_COLS;
	    c = d - 1;

	    if( b / TOTAL_COLS > 0 &&
		b / TOTAL_COLS < TOTAL_ROWS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();
	      
	      mRotation = 3;
	    }
	    else
	    {
	      b = mGridPositions[ 2 ] + TOTAL_COLS;
	      a = b - TOTAL_COLS;
	      d = b + TOTAL_COLS;
	      c = d - 1;

	      if( b / TOTAL_COLS > 0 &&
		  b / TOTAL_COLS < TOTAL_ROWS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 3;
	      }
	    }
	  }
	}
	else if( mRotation == 1 )
	{
	  c = mGridPositions[ 2 ];
	  b = c - 1;
	  a = b - TOTAL_COLS;
	  d = c + 1;

	  if( c % TOTAL_COLS > 0 &&
	      c % TOTAL_COLS < TOTAL_COLS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 0;
	  }
	  else
	  {
	    c = mGridPositions[ 2 ] - 1;
	    b = c - 1;
	    a = b - TOTAL_COLS;
	    d = c + 1;

	    if( c % TOTAL_COLS > 0 &&
		c % TOTAL_COLS < TOTAL_COLS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();
	      
	      mRotation = 0;
	    }
	    else
	    {
	      c = mGridPositions[ 2 ] + 1;
	      b = c - 1;
	      a = b - TOTAL_COLS;
	      d = c + 1; 

	      if( c % TOTAL_COLS > 0 &&
		  c % TOTAL_COLS < TOTAL_COLS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();
	      
		mRotation = 0;
	      }
	    }
	  }
	}
	else if( mRotation == 2 )
	{
	  c = mGridPositions[ 1 ];
	  a = c - TOTAL_COLS;
	  b = a + 1;
	  d = c + TOTAL_COLS;

	  if( c / TOTAL_COLS > 0 &&
	      c / TOTAL_COLS < TOTAL_ROWS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;
	    
	    draw();
	    
	    mRotation = 1;
	  }
	  else
	  {
	    c = mGridPositions[ 1 ] - TOTAL_COLS;
	    a = c - TOTAL_COLS;
	    b = a + 1;
	    d = c + TOTAL_COLS;
	    
	    if( c / TOTAL_COLS > 0 &&
		c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 1;
	    }
	    else
	    {
	      c = mGridPositions[ 1 ] + TOTAL_COLS;
	      a = c - TOTAL_COLS;
	      b = a + 1;
	      d = c + TOTAL_COLS;

	      if( c / TOTAL_COLS > 0 &&
		  c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 1;
	      }
	    }
	  }
	}
	else if( mRotation == 3 )
	{
	  b = mGridPositions[ 1 ];
	  a = b - 1;
	  c = b + 1;
	  d = c + TOTAL_COLS;

	  if( b % TOTAL_COLS > 0 &&
	      b % TOTAL_COLS < TOTAL_COLS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 2;
	  }
	  else
	  {
	    b = mGridPositions[ 1 ] - 1;
	    a = b - 1;
	    c = b + 1;
	    d = c + TOTAL_COLS;

	    if( b % TOTAL_COLS > 0 &&
		b % TOTAL_COLS < TOTAL_COLS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 2;
	    }
	    else
	    {
	      b = mGridPositions[ 1 ] + 1;
	      a = b - 1;
	      c = b + 1;
	      d = c + TOTAL_COLS;

	      if( c % TOTAL_COLS > 0 &&
		  c % TOTAL_COLS < TOTAL_COLS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 2;
	      }
	    }
	  }
	}
      }
      if( mType == TETROMINO_L )
      {
	if( mRotation == 0 )
	{
	  c = mGridPositions[ 2 ];
	  b = c - TOTAL_COLS;
	  a = b - 1;
	  d = c + TOTAL_COLS;

	  if( c / TOTAL_COLS > 0 &&
	      c / TOTAL_COLS < TOTAL_ROWS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();
	    
	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 3;
	  }
	  else
	  {
	    c = mGridPositions[ 2 ] - TOTAL_COLS;
	    b = c - TOTAL_COLS;
	    a = b - 1;
	    d = c + TOTAL_COLS;

	    if( c / TOTAL_COLS > 0 &&
		c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();
	      
	      mRotation = 3;
	    }
	    else
	    {
	      c = mGridPositions[ 2 ] + TOTAL_COLS;
	      b = c - TOTAL_COLS;
	      a = b - 1;
	      d = c + TOTAL_COLS;

	      if( c / TOTAL_COLS > 0 &&
		  c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 3;
	      }
	    }
	  }
	}
	else if( mRotation == 1 )
	{
	  c = mGridPositions[ 1 ];
	  b = c - 1;
	  d = c + 1;
	  a = d - TOTAL_COLS;

	  if( c % TOTAL_COLS > 0 &&
	      c % TOTAL_COLS < TOTAL_COLS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 0;
	  }
	  else
	  {
	    c = mGridPositions[ 1 ] - 1;
	    b = c - 1;
	    d = c + 1;
	    a = d - TOTAL_COLS;

	    if( c % TOTAL_COLS > 0 &&
		c % TOTAL_COLS < TOTAL_COLS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();
	      
	      mRotation = 0;
	    }
	    else
	    {
	      c = mGridPositions[ 1 ] + 1;
	      b = c - 1;
	      d = c + 1;
	      a = d - TOTAL_COLS;

	      if( c % TOTAL_COLS > 0 &&
		  c % TOTAL_COLS < TOTAL_COLS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();
	      
		mRotation = 0;
	      }
	    }
	  }
	}
	else if( mRotation == 2 )
	{
	  b = mGridPositions[ 1 ];
	  a = b - TOTAL_COLS;
	  c = b + TOTAL_COLS;
	  d = c + 1;

	  if( b / TOTAL_COLS > 0 &&
	      b / TOTAL_COLS < TOTAL_ROWS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;
	    
	    draw();
	    
	    mRotation = 1;
	  }
	  else
	  {
	    b = mGridPositions[ 1 ] - TOTAL_COLS;
	    a = b - TOTAL_COLS;
	    c = b + TOTAL_COLS;
	    d = c + 1;
	    
	    if( b / TOTAL_COLS > 0 &&
		b / TOTAL_COLS < TOTAL_ROWS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 1;
	    }
	    else
	    {
	      b = mGridPositions[ 1 ] + TOTAL_COLS;
	      a = b - TOTAL_COLS;
	      c = b + TOTAL_COLS;
	      d = c + 1;

	      if( b / TOTAL_COLS > 0 &&
		  b / TOTAL_COLS < TOTAL_ROWS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 1;
	      }
	    }
	  }
	}
	else if( mRotation == 3 )
	{
	  b = mGridPositions[ 2 ];
	  a = b - 1;
	  d = a + TOTAL_COLS;
	  c = b + 1;

	  if( b % TOTAL_COLS > 0 &&
	      b % TOTAL_COLS < TOTAL_COLS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 2;
	  }
	  else
	  {
	    b = mGridPositions[ 2 ] - 1;
	    a = b - 1;
	    d = a + TOTAL_COLS;
	    c = b + 1;

	    if( b % TOTAL_COLS > 0 &&
		b % TOTAL_COLS < TOTAL_COLS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 2;
	    }
	    else
	    {
	      b = mGridPositions[ 2 ] + 1;
	      a = b - 1;
	      d = a + TOTAL_COLS;
	      c = b + 1;

	      if( b % TOTAL_COLS > 0 &&
		  b % TOTAL_COLS < TOTAL_COLS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 2;
	      }
	    }
	  }
	}
      }
      if( mType == TETROMINO_O )
      {
	Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );
      }
      if( mType == TETROMINO_S )
      {
	if( mRotation == 0 )
	{
	  c = mGridPositions[ 3 ];
	  b = c - 1;
	  a = b - TOTAL_COLS;
	  d = c + TOTAL_COLS;

	  if( c / TOTAL_COLS > 0 &&
	      c / TOTAL_COLS < TOTAL_ROWS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();
	    
	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 3;
	  }
	  else
	  {
	    c = mGridPositions[ 3 ] - TOTAL_COLS;
	    b = c - 1;
	    a = b - TOTAL_COLS;
	    d = c + TOTAL_COLS;

	    if( c / TOTAL_COLS > 0 &&
		c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();
	      
	      mRotation = 3;
	    }
	    else
	    {
	      c = mGridPositions[ 3 ] + TOTAL_COLS;
	      b = c - 1;
	      a = b - TOTAL_COLS;
	      d = c + TOTAL_COLS;

	      if( c / TOTAL_COLS > 0 &&
		  c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 3;
	      }
	    }
	  }
	}
	else if( mRotation == 1 )
	{
	  d = mGridPositions[ 1 ];
	  a = d - TOTAL_COLS;
	  b = a + 1;
	  c = d - 1;

	  if( d % TOTAL_COLS > 0 &&
	      d % TOTAL_COLS < TOTAL_COLS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 0;
	  }
	  else
	  {
	    d = mGridPositions[ 1 ] - 1;
	    a = d - TOTAL_COLS;
	    b = a + 1;
	    c = d - 1;

	    if( d % TOTAL_COLS > 0 &&
		d % TOTAL_COLS < TOTAL_COLS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();
	      
	      mRotation = 0;
	    }
	    else
	    {
	      d = mGridPositions[ 1 ] + 1;
	      a = d - TOTAL_COLS;
	      b = a + 1;
	      c = d - 1;

	      if( d % TOTAL_COLS > 0 &&
		  d % TOTAL_COLS < TOTAL_COLS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();
	      
		mRotation = 0;
	      }
	    }
	  }
	}
	else if( mRotation == 2 )
	{
	  b = mGridPositions[ 0 ];
     	  a = b - TOTAL_COLS;
	  c = b + 1;
	  d = c + TOTAL_COLS;

	  if( b / TOTAL_COLS > 0 &&
	      b / TOTAL_COLS < TOTAL_ROWS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;
	    
	    draw();
	    
	    mRotation = 1;
	  }
	  else
	  {
	    b = mGridPositions[ 0 ] - TOTAL_COLS;
	    a = b - TOTAL_COLS;
	    c = b + 1;
	    d = c + TOTAL_COLS;
	    
	    if( b / TOTAL_COLS > 0 &&
		b / TOTAL_COLS < TOTAL_ROWS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 1;
	    }
	    else
	    {
	      b = mGridPositions[ 0 ] + TOTAL_COLS;
	      a = b - TOTAL_COLS;
	      c = b + 1;
	      d = c + TOTAL_COLS;

	      if( b / TOTAL_COLS > 0 &&
		  b / TOTAL_COLS < TOTAL_ROWS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 1;
	      }
	    }
	  }
	}
	else if( mRotation == 3 )
	{
	  a = mGridPositions[ 2 ];
	  b = a + 1;
	  d = a + TOTAL_COLS;
	  c = d - 1;

	  if( a % TOTAL_COLS > 0 &&
	      a % TOTAL_COLS < TOTAL_COLS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 2;
	  }
	  else
	  {
	    a = mGridPositions[ 2 ] - 1;
	    b = a + 1;
	    d = a + TOTAL_COLS;
	    c = d - 1;

	    if( a % TOTAL_COLS > 0 &&
		a % TOTAL_COLS < TOTAL_COLS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 2;
	    }
	    else
	    {
	      a = mGridPositions[ 2 ] + 1;
	      b = a + 1;
	      d = a + TOTAL_COLS;
	      c = d - 1;

	      if( a % TOTAL_COLS > 0 &&
		  a % TOTAL_COLS < TOTAL_COLS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 2;
	      }
	    }
	  }
	}
      }
      if( mType == TETROMINO_T )
      {
	if( mRotation == 0 )
	{
	  c = mGridPositions[ 2 ];
	  a = c - TOTAL_COLS;
	  b = c - 1;
	  d = c + TOTAL_COLS;

	  if( c / TOTAL_COLS > 0 &&
	      c / TOTAL_COLS < TOTAL_ROWS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();
	    
	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 3;
	  }
	  else
	  {
	    c = mGridPositions[ 2 ] - TOTAL_COLS;
	    a = c - TOTAL_COLS;
	    b = c - 1;
	    d = c + TOTAL_COLS;

	    if( c / TOTAL_COLS > 0 &&
		c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();
	      
	      mRotation = 3;
	    }
	    else
	    {
	      c = mGridPositions[ 2 ] + TOTAL_COLS;
	      a = c - TOTAL_COLS;
	      b = c - 1;
	      d = c + TOTAL_COLS;

	      if( c / TOTAL_COLS > 0 &&
		  c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 3;
	      }
	    }
	  }
	}
	else if( mRotation == 1 )
	{
	  c = mGridPositions[ 1 ];
	  b = c - 1;
	  d = c + 1;
	  a = c - TOTAL_COLS;

	  if( c % TOTAL_COLS > 0 &&
	      c % TOTAL_COLS < TOTAL_COLS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 0;
	  }
	  else
	  {
	    c = mGridPositions[ 1 ] - 1;
	    b = c - 1;
	    d = c + 1;
	    a = c - TOTAL_COLS;

	    if( c % TOTAL_COLS > 0 &&
		c % TOTAL_COLS < TOTAL_COLS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();
	      
	      mRotation = 0;
	    }
	    else
	    {
	      c = mGridPositions[ 1 ] + 1;
	      b = c - 1;
	      d = c + 1;
	      a = c - TOTAL_COLS;

	      if( c % TOTAL_COLS > 0 &&
		  c % TOTAL_COLS < TOTAL_COLS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();
	      
		mRotation = 0;
	      }
	    }
	  }
	}
	else if( mRotation == 2 )
	{
	  b = mGridPositions[ 1 ];
	  a = b - TOTAL_COLS;
	  c = b + 1;
	  d = b + TOTAL_COLS;

	  if( b / TOTAL_COLS > 0 &&
	      b / TOTAL_COLS < TOTAL_ROWS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;
	    
	    draw();
	    
	    mRotation = 1;
	  }
	  else
	  {
	    b = mGridPositions[ 1 ] - TOTAL_COLS;
	    a = b - TOTAL_COLS;
	    c = b + 1;
	    d = b + TOTAL_COLS;
	    
	    if( b / TOTAL_COLS > 0 &&
		b / TOTAL_COLS < TOTAL_ROWS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 1;
	    }
	    else
	    {
	      b = mGridPositions[ 1 ] + TOTAL_COLS;
	      a = b - TOTAL_COLS;
	      c = b + 1;
	      d = b + TOTAL_COLS;

	      if( b / TOTAL_COLS > 0 &&
		  b / TOTAL_COLS < TOTAL_ROWS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 1;
	      }
	    }
	  }
	}
	else if( mRotation == 3 )
	{
	  b = mGridPositions[ 2 ];
	  a = b - 1;
	  c = b + 1;
	  d = b + TOTAL_COLS;

	  if( b % TOTAL_COLS > 0 &&
	      b % TOTAL_COLS < TOTAL_COLS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 2;
	  }
	  else
	  {
	    b = mGridPositions[ 2 ] - 1;
	    a = b - 1;
	    c = b + 1;
	    d = b + TOTAL_COLS;

	    if( b % TOTAL_COLS > 0 &&
		b % TOTAL_COLS < TOTAL_COLS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 2;
	    }
	    else
	    {
	      b = mGridPositions[ 2 ] + 1;
	      a = b - 1;
	      c = b + 1;
	      d = b + TOTAL_COLS;

	      if( b % TOTAL_COLS > 0 &&
		  b % TOTAL_COLS < TOTAL_COLS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 2;
	      }
	    }
	  }
	}
      }
      if( mType == TETROMINO_Z )
      {
	if( mRotation == 0 )
	{
	  c = mGridPositions[ 2 ];
	  a = c - TOTAL_COLS;
	  b = c - 1;
	  d = b + TOTAL_COLS;

	  if( c / TOTAL_COLS > 0 &&
	      c / TOTAL_COLS < TOTAL_ROWS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();
	    
	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 3;
	  }
	  else
	  {
	    c = mGridPositions[ 2 ] - TOTAL_COLS;
	    a = c - TOTAL_COLS;
	    b = c - 1;
	    d = b + TOTAL_COLS;

	    if( c / TOTAL_COLS > 0 &&
		c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();
	      
	      mRotation = 3;
	    }
	    else
	    {
	      c = mGridPositions[ 2 ] + TOTAL_COLS;
	      a = c - TOTAL_COLS;
	      b = c - 1;
	      d = b + TOTAL_COLS;

	      if( c / TOTAL_COLS > 0 &&
		  c / TOTAL_COLS < TOTAL_ROWS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 3;
	      }
	    }
	  }
	}
	else if( mRotation == 1 )
	{
	  c = mGridPositions[ 1 ];
	  b = c - TOTAL_COLS;
	  a = b - 1;
	  d = c + 1;

	  if( c % TOTAL_COLS > 0 &&
	      c % TOTAL_COLS < TOTAL_COLS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 0;
	  }
	  else
	  {
	    c = mGridPositions[ 1 ] - 1;
	    b = c - TOTAL_COLS;
	    a = b - 1;
	    d = c + 1;

	    if( c % TOTAL_COLS > 0 &&
		c % TOTAL_COLS < TOTAL_COLS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();
	      
	      mRotation = 0;
	    }
	    else
	    {
	      c = mGridPositions[ 1 ] + 1;
	      b = c - TOTAL_COLS;
	      a = b - 1;
	      d = c + 1;

	      if( c % TOTAL_COLS > 0 &&
		  c % TOTAL_COLS < TOTAL_COLS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();
	      
		mRotation = 0;
	      }
	    }
	  }
	}
	else if( mRotation == 2 )
	{
	  b = mGridPositions[ 1 ];
	  c = b + 1;
	  a = c - TOTAL_COLS;
	  d = b + TOTAL_COLS;

	  if( b / TOTAL_COLS > 0 &&
	      b / TOTAL_COLS < TOTAL_ROWS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;
	    
	    draw();
	    
	    mRotation = 1;
	  }
	  else
	  {
	    b = mGridPositions[ 1 ] - TOTAL_COLS;
	    c = b + 1;
	    a = c - TOTAL_COLS;
	    d = b + TOTAL_COLS;
	    
	    if( b / TOTAL_COLS > 0 &&
		b / TOTAL_COLS < TOTAL_ROWS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 1;
	    }
	    else
	    {
	      b = mGridPositions[ 1 ] + TOTAL_COLS;
	      c = b + 1;
	      a = c - TOTAL_COLS;
	      d = b + TOTAL_COLS;

	      if( b / TOTAL_COLS > 0 &&
		  b / TOTAL_COLS < TOTAL_ROWS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 1;
	      }
	    }
	  }
	}
	else if( mRotation == 3 )
	{
	  b = mGridPositions[ 2 ];
	  a = b - 1;
	  c = b + TOTAL_COLS;
	  d = c + 1;

	  if( b % TOTAL_COLS > 0 &&
	      b % TOTAL_COLS < TOTAL_COLS - 1 &&
	      mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	      mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	  {
	    Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	    erase();

	    mGridPositions[ 0 ] = a;
	    mGridPositions[ 1 ] = b;
	    mGridPositions[ 2 ] = c;
	    mGridPositions[ 3 ] = d;

	    draw();

	    mRotation = 2;
	  }
	  else
	  {
	    b = mGridPositions[ 2 ] - 1;
	    a = b - 1;
	    c = b + TOTAL_COLS;
	    d = c + 1;

	    if( b % TOTAL_COLS > 0 &&
		b % TOTAL_COLS < TOTAL_COLS - 1 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

	      erase();

	      mGridPositions[ 0 ] = a;
	      mGridPositions[ 1 ] = b;
	      mGridPositions[ 2 ] = c;
	      mGridPositions[ 3 ] = d;

	      draw();

	      mRotation = 2;
	    }
	    else
	    {
	      b = mGridPositions[ 2 ] + 1;
	      a = b - 1;
	      c = b + TOTAL_COLS;
	      d = c + 1;

	      if( b % TOTAL_COLS > 0 &&
		  b % TOTAL_COLS < TOTAL_COLS - 1 &&
		  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
		  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	      {
		Mix_PlayChannel( MIX_CHANNEL_MOVE, gMoveSound, 0 );

		erase();

		mGridPositions[ 0 ] = a;
		mGridPositions[ 1 ] = b;
		mGridPositions[ 2 ] = c;
		mGridPositions[ 3 ] = d;

		draw();

		mRotation = 2;
	      }
	    }
	  }
	}
      }
    }

    // Move Tetromino down
    if( e.key.keysym.sym == SDLK_DOWN )
    {
      if( mInitialFallDelay > 70 )
      {
	mFallDelay = 70;
      }
      else
      {
	mFallDelay = mInitialFallDelay / 2;
      }
    }

    // Hard drop Tetromino
    if( e.key.keysym.sym == SDLK_SPACE )
    {
      erase();

      for( int i = 0; i < 4; i++ )
      {
	mGridPositions[ i ] = mGhostPositions[ i ];
      }

      draw();
      
      mFallDelay = 1;
    }

    // Hold
    if( e.key.keysym.sym == SDLK_c )
    {
      erase();

      Mix_PlayChannel( MIX_CHANNEL_HOLD, gHoldSound, 0 );
    }
  }

  if( e.type == SDL_KEYUP )
  {
    // Restore fall delay
    if( e.key.keysym.sym == SDLK_DOWN )
    {
      mFallDelay = mInitialFallDelay;
    }
  }
}

// Access type
TetrominoFlag Tetromino::getType()
{
  return mType;
}

// Reset member variables
void Tetromino::clear()
{
  mType = TETROMINO_NULL;
  
  mGridSquares = NULL;

  for( int i = 0; i < 4; i++ )
  {
    mGridPositions[ i ] = -1;
    mGhostPositions[ i ] = -1;
  }

  mRotation = 0;

  mTimer.stop();

  mFallDelay = 0;
  mInitialFallDelay = 0;
}

// Render Tetromino blocks
void Tetromino::draw()
{
  if( mGridSquares != NULL )
  {
    mGhostPositions[ 0 ] = mGridPositions[ 0 ];
    mGhostPositions[ 1 ] = mGridPositions[ 1 ];
    mGhostPositions[ 2 ] = mGridPositions[ 2 ];
    mGhostPositions[ 3 ] = mGridPositions[ 3 ];

    // Move ghost blocks down until they hit the bottom or already placed blocks
    while( mGhostPositions[ 0 ] + TOTAL_COLS < TOTAL_SQUARES &&
	   mGhostPositions[ 1 ] + TOTAL_COLS < TOTAL_SQUARES &&
	   mGhostPositions[ 2 ] + TOTAL_COLS < TOTAL_SQUARES &&
	   mGhostPositions[ 3 ] + TOTAL_COLS < TOTAL_SQUARES &&
	   mGridSquares[ mGhostPositions[ 0 ] + TOTAL_COLS ].getState() != SQUARE_STATE_STILL &&
	   mGridSquares[ mGhostPositions[ 1 ] + TOTAL_COLS ].getState() != SQUARE_STATE_STILL &&
	   mGridSquares[ mGhostPositions[ 2 ] + TOTAL_COLS ].getState() != SQUARE_STATE_STILL &&
	   mGridSquares[ mGhostPositions[ 3 ] + TOTAL_COLS ].getState() != SQUARE_STATE_STILL )
    {
      mGhostPositions[ 0 ] += TOTAL_COLS;
      mGhostPositions[ 1 ] += TOTAL_COLS;
      mGhostPositions[ 2 ] += TOTAL_COLS;
      mGhostPositions[ 3 ] += TOTAL_COLS;
    }

    // Draw ghost blocks
    for( int i = 0; i < 4; i++ )
    {
      mGridSquares[ mGhostPositions[ i ] ].fill( mType );
      mGridSquares[ mGhostPositions[ i ] ].setAlpha( 128 );
    }

    // Draw normal blocks
    for( int i = 0; i < 4; i++ )
    {
      mGridSquares[ mGridPositions[ i ] ].fill( mType );
    } 
  }
}

// Clear all Tetromino blocks
void Tetromino::erase()
{
  if( mGridSquares != NULL )
  {
    for( int i = 0; i < 4; i++ )
    {
      mGridSquares[ mGridPositions[ i ] ].clear();
      mGridSquares[ mGhostPositions[ i ] ].clear();
    }
  }
}

// Set all Tetromino blocks to stop falling
void Tetromino::land()
{
  if( mGridSquares != NULL )
  {
    Mix_PlayChannel( MIX_CHANNEL_LAND, gLandSound, 0 );

    for( int i = 0; i < 4; i++ )
    {
      mGridSquares[ mGridPositions[ i ] ].stop();
    }
  }
}
