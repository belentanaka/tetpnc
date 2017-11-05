#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <string>

#include "../constants.h"
#include "../globals/globals.h"
#include "../LTexture/LTexture.h"
#include "../textures/textures.h"
#include "../Timer/Timer.h"
#include "../Square/Square.h"
#include "../Tetromino/Tetromino.h"
#include "../GameState/GameState.h"
#include "../functions/functions.h"
#include "Play.h"

Play::Play( Stats* stats, Square* gridSquares, Square** nextSquares, Square** holdSquares, SDL_Rect* statAreas )
{
  mStats = stats;
  mStats->holdTetromino = TETROMINO_NULL;
  mStats->score = 0;
  mStats->lines = 0;
  mStats->level = 1;
  mStats->currentBG = rand() % TOTAL_BG;
  mStats->currentBGM = rand() % TOTAL_BGM;

  mGridSquares = gridSquares;
  mNextSquares = nextSquares;
  mHoldSquares = holdSquares;

  TetrominoFlag first = randomTetromino( TETROMINO_NULL );
  if( !createTetromino( first ) )
  {
    mNextState = GAME_STATE_ERROR;
    printf( "Error: Could not generate first tetromino!\n" );
  }
  else
  {
    mStats->nextTetrominoes[ 1 ] = randomTetromino( first );
    mStats->nextTetrominoes[ 2 ] = randomTetromino( mStats->nextTetrominoes[ 1 ] );
    updateNext();
  }
  
  for( int i = 0; i < 3; i++ )
  {
    mStatCenters[ i ].x = statAreas[ i ].x + ( statAreas[ i ].w / 2 );
    mStatCenters[ i ].y = statAreas[ i ].y + ( statAreas[ i ].h / 2 );
    mStatPositions[ i ] = mStatCenters[ i ];
  }

  mStarted = false;
  mPaused = false;
  mHolding = false;
  mStatsChanged = true;

  mClearing = false;
  mTetris = false;

  for( int i = 0; i < TOTAL_BG; i++ )
  {
    gBGTextures[ i ].setAlpha( 255 );
  }

  mNextState = GAME_STATE_NULL;

  mTimer.start();
}

Play::~Play()
{
  mStats = NULL;
  mGridSquares = NULL;
  mNextSquares = NULL;
  mHoldSquares = NULL;
 
  if( mTetromino != NULL )
  {
    delete mTetromino;
    mTetromino = NULL;
  }
}

void Play::handleEvent( SDL_Event& e )
{
  if( mStarted )
  {
    if( e.type == SDL_KEYDOWN )
    {
      if( e.key.keysym.sym == SDLK_ESCAPE && e.key.repeat == 0 )
      {
	if( mPaused )
	{
	  mPaused = false;
	  mTimer.unpause();
	  Mix_Resume( -1 );
	}
	else
	{
	  mPaused = true;
	  mTimer.pause();
	  Mix_Pause( -1 );
	}
      }
      else if( !mPaused && !mClearing )
      {
	if( e.key.keysym.sym == SDLK_c )
	{
	  if( !mHolding )
	  {
	    mHolding = true;

	    TetrominoFlag next;

	    if( mStats->holdTetromino == TETROMINO_NULL )
	    {
	      next = mStats->nextTetrominoes[ 0 ];
	      updateNext();
	    }
	    else
	    {
	      next = mStats->holdTetromino;
	    }

	    mStats->holdTetromino = mTetromino->getType();

	    mTetromino->handleEvent( e );

	    delete mTetromino;

	    if( !createTetromino( next ) )
	    {
	      mNextState = GAME_STATE_GAMEOVER;
	    }
	    else
	    {
	      updateHold();
	    }

	    Mix_PlayChannel( MIX_CHANNEL_HOLD, gHoldSound, 0 );
	  }
	}
	else
	{
	  mTetromino->handleEvent( e );
	}
      }
    }
    else if( e.type == SDL_KEYUP )
    {
      mTetromino->handleEvent( e );
    }
  }
}

void Play::logic()
{
  // If the game has started
  if( mStarted )
  {
    // If the game is not paused
    if( !mPaused )
    {
      // If lines are being cleared
      if( mClearing )
      {
	if( mTimer.getTicks() >= 500 )
	{
	  for( int i = 0; i < TOTAL_SQUARES; i += TOTAL_COLS )
	  {
	    bool rowFull = true;

	    for( int j = i; j < i + TOTAL_COLS; j++ )
	    {
	      if( mGridSquares[ j ].getState() != SQUARE_STATE_STILL )
	      {
		rowFull = false;
	      }
	    }

	    if( rowFull )
	    {
	      for( int j = i; j < i + TOTAL_COLS; j++ )
	      {
		mGridSquares[ j ].clear();
	      }
	    }
	  }

	  for( int i = 0; i < TOTAL_SQUARES; i += TOTAL_COLS )
	  {
	    bool rowEmpty = true;

	    for( int j = i; j < i + TOTAL_COLS; j++ )
	    {
	      if( mGridSquares[ j ].getState() == SQUARE_STATE_STILL )
	      {
		rowEmpty = false;
	      }
	    }

	    if( rowEmpty )
	    {
	      for( int j = i; j >= 0; j -= TOTAL_COLS )
	      {
		for( int k = j; k < j + TOTAL_COLS; k++ )
		{
		  if( k >= TOTAL_COLS )
		  {
		    mGridSquares[ k ].replace( mGridSquares[ k - TOTAL_COLS ] );
		  }
		  else
		  {
		    mGridSquares[ k ].clear();
		  }
		}
	      }
	    }
	  }

	  mClearing = false;
	  mTetris = false;

	  if( !createTetromino( mStats->nextTetrominoes[ 0 ] ) )
	  {
	    mNextState = GAME_STATE_GAMEOVER;
	  }
	  else
	  {
	    updateNext();
	  }
	}
      }
      // If lines are not being cleared
      else
      {
	if( mTetromino != NULL && !( mTetromino->fall() ) )
	{
	  delete mTetromino;
	  
	  mStats->score += 10 * mStats->level;
	  mStatsChanged = true;

	  if( mHolding )
	  {
	    mHolding = false;
	  }

	  int lines = 0; 
	  
	  for( int i = 0; i < TOTAL_SQUARES; i += TOTAL_COLS )
	  {
	    bool rowFilled = true;
	    
	    for( int j = i; j < i + TOTAL_COLS; j++ )
	    {
	      if( mGridSquares[ j ].getState() != SQUARE_STATE_STILL )
	      {
		rowFilled = false;
	      }
	    }

	    if( rowFilled )
	    {
	      lines++;
	    }
	  }

	  if( lines > 0 )
	  {
	    mClearing = true;
	    mStats->lines += lines;

	    switch( lines )
	    {
	      case 1:
		mStats->score += 50 * mStats->level;
		break;

  	      case 2:
		mStats->score += 150 * mStats->level;
		break;

	      case 3:
		mStats->score += 350 * mStats->level;
		break;

	      case 4:
		mStats->score += 1000 * mStats->level;
		mTetris = true;
	    }

	    if( mTetris )
	    {
	      Mix_PlayChannel( MIX_CHANNEL_TETRIS, gTetrisSound, 0 );
	    }
	    else
	    {
	      Mix_PlayChannel( MIX_CHANNEL_CLEAR, gClearSound, 0 );
	    }

	    int level = mStats->lines / 10;
	    
	    if( mStats->level < level )
	    {
	      mStats->level = level;
	      
	      int nextBGM = rand() % TOTAL_BGM;

	      while( nextBGM == mStats->currentBGM )
	      {
		nextBGM = rand() % TOTAL_BGM;
	      }

	      mStats->currentBGM = nextBGM;

	      int nextBG = rand() % TOTAL_BG;

	      while( nextBG == mStats->currentBG )
	      {
		nextBG = rand() % TOTAL_BG;
	      }

	      mStats->currentBG = nextBG;

	      Mix_FadeOutChannel( MIX_CHANNEL_MUSIC, 500 );
	    }

	    mTimer.start();
	  }
	  else
	  {
	    if( !createTetromino( mStats->nextTetrominoes[ 0 ] ) )
	    {
	      mNextState = GAME_STATE_GAMEOVER;
	    }
	    else
	    {
	      updateNext();
	    }
	  }
	}

	if( Mix_Playing( MIX_CHANNEL_MUSIC ) == 0 )
	{
	  Mix_PlayChannel( MIX_CHANNEL_MUSIC, gBGMusic[ mStats->currentBGM ], - 1 );
	}
      }
    }
  }
  // If the game has not started
  else
  {
    if( mTimer.getTicks() >= 3000 )
    {
      mStarted = true;
      mTimer.start();
    }

    if( Mix_Playing( MIX_CHANNEL_MUSIC ) == 1 )
    {
      Mix_FadeOutChannel( MIX_CHANNEL_MUSIC, 2000 );
    }
  }
}

void Play::render()
{
  int currentTicks = mTimer.getTicks();

  // If the game has started
  if( mStarted )
  {
    gBGTextures[ mStats->currentBG ].render( 0, 0 );

    // If lines are being cleared
    if( mClearing )
    {
      gBlackTexture.setAlpha( 255 * currentTicks / 500 );
      gBlackTexture.render( 0, 0 );

      int frame = 4;

      if( currentTicks < 250 )
      {
	frame = currentTicks / 50;
      }

      gHandWhiteTexture.render( 0, 0, &gHandClips[ frame ] );
      gHandWhiteTexture.render( 0, 275, &gHandClips[ frame ] );

      if( currentTicks < 500 )
      {
	for( int i = 0; i < TOTAL_SQUARES; i += TOTAL_COLS )
	{
	  bool rowFilled = true;

	  for( int j = i; j < i + TOTAL_COLS; j++ )
	  {
	    if( mGridSquares[ j ].getState() != SQUARE_STATE_STILL )
	    {
	      rowFilled = false;
	    }
	  }

	  if( rowFilled )
	  {
	    for( int j = i; j < i + TOTAL_COLS; j++ )
	    {
	      mGridSquares[ j ].setAlpha( 255 - ( 255 * currentTicks / 500 ) );
	    }
	  }
	}
      }
    }
      
    gPlayBGTexture.render( 0, 0 );

    // If player got a tetris
    if( mTetris )
    {
      gTetrisTexture.setAlpha( 255 * ( currentTicks % 100 ) / 100 );
      gTetrisTexture.render( 0, 0 );
    }

    SDL_Color textColor = { 0, 0, 0 };

    // If stats need to be updated
    if( mStatsChanged )
    {
      std::string score = std::to_string( mStats->score );
      gScoreTextTexture.loadFromRenderedText( score.c_str(), textColor );
      mStatPositions[ 0 ].x = mStatCenters[ 0 ].x - ( gScoreTextTexture.getWidth() / 2 );
      mStatPositions[ 0 ].y = mStatCenters[ 0 ].y - ( gScoreTextTexture.getHeight() / 2 );

      std::string lines = std::to_string( mStats->lines );
      gLinesTextTexture.loadFromRenderedText( lines.c_str(), textColor );
      mStatPositions[ 1 ].x = mStatCenters[ 1 ].x - ( gLinesTextTexture.getWidth() / 2 );
      mStatPositions[ 1 ].y = mStatCenters[ 1 ].y - ( gLinesTextTexture.getHeight() / 2 );

      std::string level = std::to_string( mStats->level );
      gLevelTextTexture.loadFromRenderedText( level.c_str(), textColor );
      mStatPositions[ 2 ].x = mStatCenters[ 2 ].x - ( gLevelTextTexture.getWidth() / 2 );
      mStatPositions[ 2 ].y = mStatCenters[ 2 ].y - ( gLevelTextTexture.getHeight() / 2 );

      mStatsChanged = false;
    }

    gScoreTextTexture.render( mStatPositions[ 0 ].x, mStatPositions[ 0 ].y );
    gLinesTextTexture.render( mStatPositions[ 1 ].x, mStatPositions[ 1 ].y );
    gLevelTextTexture.render( mStatPositions[ 2 ].x, mStatPositions[ 2 ].y );

    switch( mStats->nextTetrominoes[ 0 ] )
    {
      case TETROMINO_I:
	mNextSquares[ 0 ][ 4 ].render();
	mNextSquares[ 0 ][ 5 ].render();
	mNextSquares[ 0 ][ 6 ].render();
	mNextSquares[ 0 ][ 7 ].render();
	break;

      case TETROMINO_J:
	mNextSquares[ 1 ][ 0 ].render();
	mNextSquares[ 1 ][ 3 ].render();
	mNextSquares[ 1 ][ 4 ].render();
	mNextSquares[ 1 ][ 5 ].render();
	break;

      case TETROMINO_L:
	mNextSquares[ 1 ][ 2 ].render();
	mNextSquares[ 1 ][ 3 ].render();
	mNextSquares[ 1 ][ 4 ].render();
	mNextSquares[ 1 ][ 5 ].render();
	break;

      case TETROMINO_O:
	mNextSquares[ 0 ][ 1 ].render();
	mNextSquares[ 0 ][ 2 ].render();
	mNextSquares[ 0 ][ 5 ].render();
	mNextSquares[ 0 ][ 6 ].render();
	break;

      case TETROMINO_S:
	mNextSquares[ 1 ][ 1 ].render();
	mNextSquares[ 1 ][ 2 ].render();
	mNextSquares[ 1 ][ 3 ].render();
	mNextSquares[ 1 ][ 4 ].render();
	break;

      case TETROMINO_T:
	mNextSquares[ 1 ][ 1 ].render();
	mNextSquares[ 1 ][ 3 ].render();
	mNextSquares[ 1 ][ 4 ].render();
	mNextSquares[ 1 ][ 5 ].render();
	break;

      case TETROMINO_Z:
	mNextSquares[ 1 ][ 0 ].render();
	mNextSquares[ 1 ][ 1 ].render();
	mNextSquares[ 1 ][ 4 ].render();
	mNextSquares[ 1 ][ 5 ].render();
	break;

      default:
	mNextState = GAME_STATE_ERROR;
    }

    switch( mStats->nextTetrominoes[ 1 ] )
    {
      case TETROMINO_I:
	mNextSquares[ 2 ][ 4 ].render();
	mNextSquares[ 2 ][ 5 ].render();
	mNextSquares[ 2 ][ 6 ].render();
	mNextSquares[ 2 ][ 7 ].render();
	break;

      case TETROMINO_J:
	mNextSquares[ 3 ][ 0 ].render();
	mNextSquares[ 3 ][ 3 ].render();
	mNextSquares[ 3 ][ 4 ].render();
	mNextSquares[ 3 ][ 5 ].render();
	break;

      case TETROMINO_L:
	mNextSquares[ 3 ][ 2 ].render();
	mNextSquares[ 3 ][ 3 ].render();
	mNextSquares[ 3 ][ 4 ].render();
	mNextSquares[ 3 ][ 5 ].render();
	break;

      case TETROMINO_O:
	mNextSquares[ 2 ][ 1 ].render();
	mNextSquares[ 2 ][ 2 ].render();
	mNextSquares[ 2 ][ 5 ].render();
	mNextSquares[ 2 ][ 6 ].render();
	break;

      case TETROMINO_S:
	mNextSquares[ 3 ][ 1 ].render();
	mNextSquares[ 3 ][ 2 ].render();
	mNextSquares[ 3 ][ 3 ].render();
	mNextSquares[ 3 ][ 4 ].render();
	break;

      case TETROMINO_T:
	mNextSquares[ 3 ][ 1 ].render();
	mNextSquares[ 3 ][ 3 ].render();
	mNextSquares[ 3 ][ 4 ].render();
	mNextSquares[ 3 ][ 5 ].render();
	break; 

      case TETROMINO_Z:
	mNextSquares[ 3 ][ 0 ].render();
	mNextSquares[ 3 ][ 1 ].render();
	mNextSquares[ 3 ][ 4 ].render();
	mNextSquares[ 3 ][ 5 ].render();
	break;

      default:
	mNextState = GAME_STATE_ERROR;
    }

    switch( mStats->nextTetrominoes[ 2 ] )
    {
      case TETROMINO_I:
	mNextSquares[ 4 ][ 4 ].render();
	mNextSquares[ 4 ][ 5 ].render();
	mNextSquares[ 4 ][ 6 ].render();
	mNextSquares[ 4 ][ 7 ].render();
	break;

      case TETROMINO_J:
	mNextSquares[ 5 ][ 0 ].render();
	mNextSquares[ 5 ][ 3 ].render();
	mNextSquares[ 5 ][ 4 ].render();
	mNextSquares[ 5 ][ 5 ].render();
	break;

      case TETROMINO_L:
	mNextSquares[ 5 ][ 2 ].render();
	mNextSquares[ 5 ][ 3 ].render();
	mNextSquares[ 5 ][ 4 ].render();
	mNextSquares[ 5 ][ 5 ].render();
	break;

      case TETROMINO_O:
	mNextSquares[ 4 ][ 1 ].render();
	mNextSquares[ 4 ][ 2 ].render();
	mNextSquares[ 4 ][ 5 ].render();
	mNextSquares[ 4 ][ 6 ].render();
	break;

      case TETROMINO_S:
	mNextSquares[ 5 ][ 1 ].render();
	mNextSquares[ 5 ][ 2 ].render();
	mNextSquares[ 5 ][ 3 ].render();
	mNextSquares[ 5 ][ 4 ].render();
	break;

      case TETROMINO_T:
	mNextSquares[ 5 ][ 1 ].render();
	mNextSquares[ 5 ][ 3 ].render();
	mNextSquares[ 5 ][ 4 ].render();
	mNextSquares[ 5 ][ 5 ].render();
	break;

      case TETROMINO_Z:
	mNextSquares[ 5 ][ 0 ].render();
	mNextSquares[ 5 ][ 1 ].render();
	mNextSquares[ 5 ][ 4 ].render();
	mNextSquares[ 5 ][ 5 ].render();
	break;

      default:
	mNextState = GAME_STATE_ERROR;
    }

    switch( mStats->holdTetromino )
    {
      case TETROMINO_NULL:
	break;

      case TETROMINO_I:
	mHoldSquares[ 0 ][ 4 ].render();
	mHoldSquares[ 0 ][ 5 ].render();
	mHoldSquares[ 0 ][ 6 ].render();
	mHoldSquares[ 0 ][ 7 ].render();
	break;

      case TETROMINO_J:
	mHoldSquares[ 1 ][ 0 ].render();
	mHoldSquares[ 1 ][ 3 ].render();
	mHoldSquares[ 1 ][ 4 ].render();
	mHoldSquares[ 1 ][ 5 ].render();
	break;

      case TETROMINO_L:
	mHoldSquares[ 1 ][ 2 ].render();
	mHoldSquares[ 1 ][ 3 ].render();
	mHoldSquares[ 1 ][ 4 ].render();
	mHoldSquares[ 1 ][ 5 ].render();
	break;

      case TETROMINO_O:
	mHoldSquares[ 0 ][ 1 ].render();
	mHoldSquares[ 0 ][ 2 ].render();
	mHoldSquares[ 0 ][ 5 ].render();
	mHoldSquares[ 0 ][ 6 ].render();
	break;

      case TETROMINO_S:
	mHoldSquares[ 1 ][ 1 ].render();
	mHoldSquares[ 1 ][ 2 ].render();
	mHoldSquares[ 1 ][ 3 ].render();
	mHoldSquares[ 1 ][ 4 ].render();
	break;

      case TETROMINO_T:
	mHoldSquares[ 1 ][ 1 ].render();
	mHoldSquares[ 1 ][ 3 ].render();
	mHoldSquares[ 1 ][ 4 ].render();
	mHoldSquares[ 1 ][ 5 ].render();
	break;

      case TETROMINO_Z:
	mHoldSquares[ 1 ][ 0 ].render();
	mHoldSquares[ 1 ][ 1 ].render();
	mHoldSquares[ 1 ][ 4 ].render();
	mHoldSquares[ 1 ][ 5 ].render();
	break;

      default:
	mNextState = GAME_STATE_ERROR;
    } 

    for( int i = ( 2 * TOTAL_COLS ); i < TOTAL_SQUARES; i++ )
    {
      if( mGridSquares[ i ].getState() != SQUARE_STATE_BLANK )
      {
	mGridSquares[ i ].render();
      }
    }

    // If the game is paused
    if( mPaused )
    {
      int pauseTicks = mTimer.getPauseTicks();
      gPausedTexture.setAlpha( 255 - ( 255 * ( ( pauseTicks % 2000 ) - 1000 ) * ( ( pauseTicks % 2000 ) - 1000 ) / 1000000 ) );
      gPausedTexture.render( 0, 0 );
    }
  }
  // If the game has not started
  else 
  {
    gBGTextures[ mStats->currentBG ].setAlpha( 255 * currentTicks / 3000 );
    gBGTextures[ mStats->currentBG ].render( 0, 0 );

    gPlayBGTexture.setAlpha( 255 * currentTicks / 3000 );
    gPlayBGTexture.render( 0, 0 );
  }
}

bool Play::createTetromino( TetrominoFlag type )
{
  bool success = true;

  int delay;

  int level = mStats->level;

  switch( level )
  {
    case 1:
      delay = 800;
      break;

    case 2:
      delay = 720;
      break;

    case 3:
      delay = 630;
      break;

    case 4:
      delay = 550;
      break;

    case 5:
      delay = 470;
      break;

    case 6:
      delay = 380;
      break;

    case 7:
      delay = 300;
      break;

    case 8:
      delay = 220;
      break;

    case 9:
      delay = 130;
      break;

    case 10:
      delay = 100;
      break;

    case 11:
    case 12:
    case 13:
      delay = 80;
      break;

    case 14:
    case 15:
    case 16:
      delay = 70;
      break;

    case 17:
    case 18:
    case 19:
      delay = 50;
      break;

    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
      delay = 30;
      break;

    default:
      delay = 20;
  }

  mTetromino = new Tetromino( type, mGridSquares, delay );
  if( mTetromino->getType() == TETROMINO_NULL )
  {
    delete mTetromino;
    success = false;
  }

  return success;
}

void Play::updateNext()
{
  for( int i = 0; i < 8; i++ )
  {
    mNextSquares[ 0 ][ i ].clear();
    mNextSquares[ 2 ][ i ].clear();
    mNextSquares[ 4 ][ i ].clear();
  }
  for( int i = 0; i < 6; i++ )
  {
    mNextSquares[ 1 ][ i ].clear();
    mNextSquares[ 3 ][ i ].clear();
    mNextSquares[ 5 ][ i ].clear();
  }

  mStats->nextTetrominoes[ 0 ] = mStats->nextTetrominoes[ 1 ];
  mStats->nextTetrominoes[ 1 ] = mStats->nextTetrominoes[ 2 ];
  mStats->nextTetrominoes[ 2 ] = randomTetromino( mStats->nextTetrominoes[ 1 ] );

  switch( mStats->nextTetrominoes[ 0 ] )
  {
    case TETROMINO_I:
      mNextSquares[ 0 ][ 4 ].fill( TETROMINO_I );
      mNextSquares[ 0 ][ 5 ].fill( TETROMINO_I );
      mNextSquares[ 0 ][ 6 ].fill( TETROMINO_I );
      mNextSquares[ 0 ][ 7 ].fill( TETROMINO_I );
      break;

    case TETROMINO_J:
      mNextSquares[ 1 ][ 0 ].fill( TETROMINO_J );
      mNextSquares[ 1 ][ 3 ].fill( TETROMINO_J );
      mNextSquares[ 1 ][ 4 ].fill( TETROMINO_J );
      mNextSquares[ 1 ][ 5 ].fill( TETROMINO_J );
      break;

    case TETROMINO_L:
      mNextSquares[ 1 ][ 2 ].fill( TETROMINO_L );
      mNextSquares[ 1 ][ 3 ].fill( TETROMINO_L );
      mNextSquares[ 1 ][ 4 ].fill( TETROMINO_L );
      mNextSquares[ 1 ][ 5 ].fill( TETROMINO_L );
      break;

    case TETROMINO_O:
      mNextSquares[ 0 ][ 1 ].fill( TETROMINO_O );
      mNextSquares[ 0 ][ 2 ].fill( TETROMINO_O );
      mNextSquares[ 0 ][ 5 ].fill( TETROMINO_O );
      mNextSquares[ 0 ][ 6 ].fill( TETROMINO_O );
      break;

    case TETROMINO_S:
      mNextSquares[ 1 ][ 1 ].fill( TETROMINO_S );
      mNextSquares[ 1 ][ 2 ].fill( TETROMINO_S );
      mNextSquares[ 1 ][ 3 ].fill( TETROMINO_S );
      mNextSquares[ 1 ][ 4 ].fill( TETROMINO_S );
      break;

    case TETROMINO_T:
      mNextSquares[ 1 ][ 1 ].fill( TETROMINO_T );
      mNextSquares[ 1 ][ 3 ].fill( TETROMINO_T );
      mNextSquares[ 1 ][ 4 ].fill( TETROMINO_T );
      mNextSquares[ 1 ][ 5 ].fill( TETROMINO_T );
      break;

    case TETROMINO_Z:
      mNextSquares[ 1 ][ 0 ].fill( TETROMINO_Z );
      mNextSquares[ 1 ][ 1 ].fill( TETROMINO_Z );
      mNextSquares[ 1 ][ 4 ].fill( TETROMINO_Z );
      mNextSquares[ 1 ][ 5 ].fill( TETROMINO_Z );
      break;

    default:
      mNextState = GAME_STATE_ERROR;
  }

  switch( mStats->nextTetrominoes[ 1 ] )
  {
    case TETROMINO_I:
      mNextSquares[ 2 ][ 4 ].fill( TETROMINO_I );
      mNextSquares[ 2 ][ 5 ].fill( TETROMINO_I );
      mNextSquares[ 2 ][ 6 ].fill( TETROMINO_I );
      mNextSquares[ 2 ][ 7 ].fill( TETROMINO_I );
      break;

    case TETROMINO_J:
      mNextSquares[ 3 ][ 0 ].fill( TETROMINO_J );
      mNextSquares[ 3 ][ 3 ].fill( TETROMINO_J );
      mNextSquares[ 3 ][ 4 ].fill( TETROMINO_J );
      mNextSquares[ 3 ][ 5 ].fill( TETROMINO_J );
      break;

    case TETROMINO_L:
      mNextSquares[ 3 ][ 2 ].fill( TETROMINO_L );
      mNextSquares[ 3 ][ 3 ].fill( TETROMINO_L );
      mNextSquares[ 3 ][ 4 ].fill( TETROMINO_L );
      mNextSquares[ 3 ][ 5 ].fill( TETROMINO_L );
      break;

    case TETROMINO_O:
      mNextSquares[ 2 ][ 1 ].fill( TETROMINO_O );
      mNextSquares[ 2 ][ 2 ].fill( TETROMINO_O );
      mNextSquares[ 2 ][ 5 ].fill( TETROMINO_O );
      mNextSquares[ 2 ][ 6 ].fill( TETROMINO_O );
      break;

    case TETROMINO_S:
      mNextSquares[ 3 ][ 1 ].fill( TETROMINO_S );
      mNextSquares[ 3 ][ 2 ].fill( TETROMINO_S );
      mNextSquares[ 3 ][ 3 ].fill( TETROMINO_S );
      mNextSquares[ 3 ][ 4 ].fill( TETROMINO_S );
      break;

    case TETROMINO_T:
      mNextSquares[ 3 ][ 1 ].fill( TETROMINO_T );
      mNextSquares[ 3 ][ 3 ].fill( TETROMINO_T );
      mNextSquares[ 3 ][ 4 ].fill( TETROMINO_T );
      mNextSquares[ 3 ][ 5 ].fill( TETROMINO_T );
      break;

    case TETROMINO_Z:
      mNextSquares[ 3 ][ 0 ].fill( TETROMINO_Z );
      mNextSquares[ 3 ][ 1 ].fill( TETROMINO_Z );
      mNextSquares[ 3 ][ 4 ].fill( TETROMINO_Z );
      mNextSquares[ 3 ][ 5 ].fill( TETROMINO_Z );
      break;

    default:
      mNextState = GAME_STATE_ERROR;
  }

  switch( mStats->nextTetrominoes[ 2 ] )
  {
    case TETROMINO_I:
      mNextSquares[ 4 ][ 4 ].fill( TETROMINO_I );
      mNextSquares[ 4 ][ 5 ].fill( TETROMINO_I );
      mNextSquares[ 4 ][ 6 ].fill( TETROMINO_I );
      mNextSquares[ 4 ][ 7 ].fill( TETROMINO_I );
      break;

    case TETROMINO_J:
      mNextSquares[ 5 ][ 0 ].fill( TETROMINO_J );
      mNextSquares[ 5 ][ 3 ].fill( TETROMINO_J );
      mNextSquares[ 5 ][ 4 ].fill( TETROMINO_J );
      mNextSquares[ 5 ][ 5 ].fill( TETROMINO_J );
      break;

    case TETROMINO_L:
      mNextSquares[ 5 ][ 2 ].fill( TETROMINO_L );
      mNextSquares[ 5 ][ 3 ].fill( TETROMINO_L );
      mNextSquares[ 5 ][ 4 ].fill( TETROMINO_L );
      mNextSquares[ 5 ][ 5 ].fill( TETROMINO_L );
      break;

    case TETROMINO_O:
      mNextSquares[ 4 ][ 1 ].fill( TETROMINO_O );
      mNextSquares[ 4 ][ 2 ].fill( TETROMINO_O );
      mNextSquares[ 4 ][ 5 ].fill( TETROMINO_O );
      mNextSquares[ 4 ][ 6 ].fill( TETROMINO_O );
      break;

    case TETROMINO_S:
      mNextSquares[ 5 ][ 1 ].fill( TETROMINO_S );
      mNextSquares[ 5 ][ 2 ].fill( TETROMINO_S );
      mNextSquares[ 5 ][ 3 ].fill( TETROMINO_S );
      mNextSquares[ 5 ][ 4 ].fill( TETROMINO_S );
      break;

    case TETROMINO_T:
      mNextSquares[ 5 ][ 1 ].fill( TETROMINO_T );
      mNextSquares[ 5 ][ 3 ].fill( TETROMINO_T );
      mNextSquares[ 5 ][ 4 ].fill( TETROMINO_T );
      mNextSquares[ 5 ][ 5 ].fill( TETROMINO_T );
      break;

    case TETROMINO_Z:
      mNextSquares[ 5 ][ 0 ].fill( TETROMINO_Z );
      mNextSquares[ 5 ][ 1 ].fill( TETROMINO_Z );
      mNextSquares[ 5 ][ 4 ].fill( TETROMINO_Z );
      mNextSquares[ 5 ][ 5 ].fill( TETROMINO_Z );
      break;

    default:
      mNextState = GAME_STATE_ERROR;
  }
}

void Play::updateHold()
{
  for( int i = 0; i < 8; i++ )
  {
    mHoldSquares[ 0 ][ i ].clear();
  }
  for( int i = 0; i < 6; i++ )
  {
    mHoldSquares[ 1 ][ i ].clear();
  }

  switch( mStats->holdTetromino )
  {
    case TETROMINO_NULL:
      break;

    case TETROMINO_I:
      mHoldSquares[ 0 ][ 4 ].fill( TETROMINO_I );
      mHoldSquares[ 0 ][ 5 ].fill( TETROMINO_I );
      mHoldSquares[ 0 ][ 6 ].fill( TETROMINO_I );
      mHoldSquares[ 0 ][ 7 ].fill( TETROMINO_I );
      break;

    case TETROMINO_J:
      mHoldSquares[ 1 ][ 0 ].fill( TETROMINO_J );
      mHoldSquares[ 1 ][ 3 ].fill( TETROMINO_J );
      mHoldSquares[ 1 ][ 4 ].fill( TETROMINO_J );
      mHoldSquares[ 1 ][ 5 ].fill( TETROMINO_J );
      break;

    case TETROMINO_L:
      mHoldSquares[ 1 ][ 2 ].fill( TETROMINO_L );
      mHoldSquares[ 1 ][ 3 ].fill( TETROMINO_L );
      mHoldSquares[ 1 ][ 4 ].fill( TETROMINO_L );
      mHoldSquares[ 1 ][ 5 ].fill( TETROMINO_L );
      break;

    case TETROMINO_O:
      mHoldSquares[ 0 ][ 1 ].fill( TETROMINO_O );
      mHoldSquares[ 0 ][ 2 ].fill( TETROMINO_O );
      mHoldSquares[ 0 ][ 5 ].fill( TETROMINO_O );
      mHoldSquares[ 0 ][ 6 ].fill( TETROMINO_O );
      break;

    case TETROMINO_S:
      mHoldSquares[ 1 ][ 1 ].fill( TETROMINO_S );
      mHoldSquares[ 1 ][ 2 ].fill( TETROMINO_S );
      mHoldSquares[ 1 ][ 3 ].fill( TETROMINO_S );
      mHoldSquares[ 1 ][ 4 ].fill( TETROMINO_S );
      break;

    case TETROMINO_T:
      mHoldSquares[ 1 ][ 1 ].fill( TETROMINO_T );
      mHoldSquares[ 1 ][ 3 ].fill( TETROMINO_T );
      mHoldSquares[ 1 ][ 4 ].fill( TETROMINO_T );
      mHoldSquares[ 1 ][ 5 ].fill( TETROMINO_T );
      break;

    case TETROMINO_Z:
      mHoldSquares[ 1 ][ 0 ].fill( TETROMINO_Z );
      mHoldSquares[ 1 ][ 1 ].fill( TETROMINO_Z );
      mHoldSquares[ 1 ][ 4 ].fill( TETROMINO_Z );
      mHoldSquares[ 1 ][ 5 ].fill( TETROMINO_Z );
      break;

    default:
      mNextState = GAME_STATE_ERROR;
  }
}
