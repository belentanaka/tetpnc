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
#include "../GameState/GameState.h"
#include "Intro.h"

Intro::Intro( Square** introSquares, SDL_Rect& listArea )
{
  mIntroSquares = introSquares;

  SDL_Color nameColor = { 255, 255, 255 };
  SDL_Color scoreColor = { 0, 0, 0 };

  for( int i = 0; i < TOTAL_SCORES; i++ )
  {
    std::string name = gScores[ i ].name;

    if( name == "" )
    {
      name = " ";
    }

    gListTextTextures[ i ].loadFromRenderedText( name, nameColor );
    gListTextTextures[ i ].setAlpha( 0 );
    std::string score = std::to_string( gScores[ i ].score );
    gListTextTextures[ i + TOTAL_SCORES ].loadFromRenderedText( score.c_str(), scoreColor );
    gListTextTextures[ i + TOTAL_SCORES ].setAlpha( 0 );
    mListCenters[ i ].x = listArea.x + ( listArea.w / 2 );
    mListCenters[ i ].y = listArea.y + ( i * gListClips[ 0 ].h ) + ( gListClips[ 0 ].h / 2 );
    mListPositions[ i ].x = mListCenters[ i ].x - ( gListTextTextures[ i ].getWidth() / 2 );
    mListPositions[ i ].y = mListCenters[ i ].y - ( gListTextTextures[ i ].getHeight() / 2 );
  }
  
  mNextState = GAME_STATE_NULL;
  
  mTimer.start();
}

Intro::~Intro()
{
  mIntroSquares = NULL;
}

void Intro::handleEvent( SDL_Event& e )
{
  if( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN )
  {
    mNextState = GAME_STATE_PLAY;
  }
}

void Intro::logic()
{
  int currentTicks = mTimer.getTicks();

  if( mTimer.getTicks() >= 20000 )
  {
    mTimer.start();
  }
    
  if( Mix_Playing( MIX_CHANNEL_MUSIC ) == 0 )
  {
    Mix_PlayChannel( MIX_CHANNEL_MUSIC, gStartMusic, -1 );
  }
}

void Intro::render()
{   
  for( int i = 0; i < 8; i++ )
  {
    mIntroSquares[ 0 ][ i ].clear();
    mIntroSquares[ 2 ][ i ].clear();
  }

  for( int i = 0; i < 6; i++ )
  {
    mIntroSquares[ 1 ][ i ].clear();
    mIntroSquares[ 3 ][ i ].clear();
  }

  gBlankBGTexture.render( 0, 0 );
   
  int alpha = 255;
  int frame = 4;

  int currentTicks = mTimer.getTicks();

  if( currentTicks < 10000 )
  {
    if( currentTicks < 250 )
    {
      alpha = 255 * currentTicks / 250;
      frame = currentTicks / 50;
    }
    else if( currentTicks >= 9750 )
    {
      alpha = 255 - ( 255 * ( currentTicks - 9750 ) / 250 );
      frame = 4 - ( ( currentTicks - 9750 ) / 50 );
    }

    gHandBlackTexture.setAlpha( alpha );
    gHandBlackTexture.render( 0, ( SCREEN_HEIGHT / 2 ) - 138, &gHandClips[ frame ] );

    if( currentTicks < 2000 )
    {
      alpha = 255 * currentTicks / 2000;
    }
    else if( currentTicks > 8000 )
    {
      alpha = 255 - ( 255 * ( currentTicks - 8000 ) / 2000 );
    } 

    int type = ( currentTicks / 100 ) % TETROMINO_COUNT;

    switch( type )
    {
      case TETROMINO_I:
	mIntroSquares[ 0 ][ 4 ].fill( TETROMINO_I );
	mIntroSquares[ 0 ][ 5 ].fill( TETROMINO_I );
	mIntroSquares[ 0 ][ 6 ].fill( TETROMINO_I );
	mIntroSquares[ 0 ][ 7 ].fill( TETROMINO_I );
      
	mIntroSquares[ 2 ][ 4 ].fill( TETROMINO_I );
	mIntroSquares[ 2 ][ 5 ].fill( TETROMINO_I );
	mIntroSquares[ 2 ][ 6 ].fill( TETROMINO_I );
	mIntroSquares[ 2 ][ 7 ].fill( TETROMINO_I );

	mIntroSquares[ 0 ][ 4 ].setAlpha( alpha );
	mIntroSquares[ 0 ][ 5 ].setAlpha( alpha );
	mIntroSquares[ 0 ][ 6 ].setAlpha( alpha );
	mIntroSquares[ 0 ][ 7 ].setAlpha( alpha );
	
	mIntroSquares[ 2 ][ 4 ].setAlpha( alpha );
	mIntroSquares[ 2 ][ 5 ].setAlpha( alpha );
	mIntroSquares[ 2 ][ 6 ].setAlpha( alpha );
	mIntroSquares[ 2 ][ 7 ].setAlpha( alpha );
	
	mIntroSquares[ 0 ][ 4 ].render();
	mIntroSquares[ 0 ][ 5 ].render();
	mIntroSquares[ 0 ][ 6 ].render();
	mIntroSquares[ 0 ][ 7 ].render();

	mIntroSquares[ 2 ][ 4 ].render();
	mIntroSquares[ 2 ][ 5 ].render();
	mIntroSquares[ 2 ][ 6 ].render();
	mIntroSquares[ 2 ][ 7 ].render();

	break;

      case TETROMINO_J:
	mIntroSquares[ 1 ][ 0 ].fill( TETROMINO_J );
	mIntroSquares[ 1 ][ 3 ].fill( TETROMINO_J );
	mIntroSquares[ 1 ][ 4 ].fill( TETROMINO_J );
	mIntroSquares[ 1 ][ 5 ].fill( TETROMINO_J );
	
	mIntroSquares[ 3 ][ 0 ].fill( TETROMINO_J );
	mIntroSquares[ 3 ][ 3 ].fill( TETROMINO_J );
	mIntroSquares[ 3 ][ 4 ].fill( TETROMINO_J );
	mIntroSquares[ 3 ][ 5 ].fill( TETROMINO_J );
	
	mIntroSquares[ 1 ][ 0 ].setAlpha( alpha );
	mIntroSquares[ 1 ][ 3 ].setAlpha( alpha );
	mIntroSquares[ 1 ][ 4 ].setAlpha( alpha );
	mIntroSquares[ 1 ][ 5 ].setAlpha( alpha );

	mIntroSquares[ 3 ][ 0 ].setAlpha( alpha );
	mIntroSquares[ 3 ][ 3 ].setAlpha( alpha );
	mIntroSquares[ 3 ][ 4 ].setAlpha( alpha );
	mIntroSquares[ 3 ][ 5 ].setAlpha( alpha );

	mIntroSquares[ 1 ][ 0 ].render();
	mIntroSquares[ 1 ][ 3 ].render();
	mIntroSquares[ 1 ][ 4 ].render();
	mIntroSquares[ 1 ][ 5 ].render();

	mIntroSquares[ 3 ][ 0 ].render();
	mIntroSquares[ 3 ][ 3 ].render();
	mIntroSquares[ 3 ][ 4 ].render();
	mIntroSquares[ 3 ][ 5 ].render();

	break;

      case TETROMINO_L:
	mIntroSquares[ 1 ][ 2 ].fill( TETROMINO_L );
	mIntroSquares[ 1 ][ 3 ].fill( TETROMINO_L );
	mIntroSquares[ 1 ][ 4 ].fill( TETROMINO_L );
	mIntroSquares[ 1 ][ 5 ].fill( TETROMINO_L );

	mIntroSquares[ 3 ][ 2 ].fill( TETROMINO_L );
	mIntroSquares[ 3 ][ 3 ].fill( TETROMINO_L );
	mIntroSquares[ 3 ][ 4 ].fill( TETROMINO_L );
	mIntroSquares[ 3 ][ 5 ].fill( TETROMINO_L );

	mIntroSquares[ 1 ][ 2 ].setAlpha( alpha );
	mIntroSquares[ 1 ][ 3 ].setAlpha( alpha );
	mIntroSquares[ 1 ][ 4 ].setAlpha( alpha );
	mIntroSquares[ 1 ][ 5 ].setAlpha( alpha );

	mIntroSquares[ 3 ][ 2 ].setAlpha( alpha );
	mIntroSquares[ 3 ][ 3 ].setAlpha( alpha );
	mIntroSquares[ 3 ][ 4 ].setAlpha( alpha );
	mIntroSquares[ 3 ][ 5 ].setAlpha( alpha );
	  
	mIntroSquares[ 1 ][ 2 ].render();
	mIntroSquares[ 1 ][ 3 ].render();
	mIntroSquares[ 1 ][ 4 ].render();
	mIntroSquares[ 1 ][ 5 ].render();

	mIntroSquares[ 3 ][ 2 ].render();
	mIntroSquares[ 3 ][ 3 ].render();
	mIntroSquares[ 3 ][ 4 ].render();
	mIntroSquares[ 3 ][ 5 ].render();

	break;

      case TETROMINO_O:
	mIntroSquares[ 0 ][ 1 ].fill( TETROMINO_O );
	mIntroSquares[ 0 ][ 2 ].fill( TETROMINO_O );
	mIntroSquares[ 0 ][ 5 ].fill( TETROMINO_O );
	mIntroSquares[ 0 ][ 6 ].fill( TETROMINO_O );
	
	mIntroSquares[ 2 ][ 1 ].fill( TETROMINO_O );
	mIntroSquares[ 2 ][ 2 ].fill( TETROMINO_O );
	mIntroSquares[ 2 ][ 5 ].fill( TETROMINO_O );
	mIntroSquares[ 2 ][ 6 ].fill( TETROMINO_O );

	mIntroSquares[ 0 ][ 1 ].setAlpha( alpha );
	mIntroSquares[ 0 ][ 2 ].setAlpha( alpha );
	mIntroSquares[ 0 ][ 5 ].setAlpha( alpha );
	mIntroSquares[ 0 ][ 6 ].setAlpha( alpha );
	  
	mIntroSquares[ 2 ][ 1 ].setAlpha( alpha );
	mIntroSquares[ 2 ][ 2 ].setAlpha( alpha );
	mIntroSquares[ 2 ][ 5 ].setAlpha( alpha );
	mIntroSquares[ 2 ][ 6 ].setAlpha( alpha );

	mIntroSquares[ 0 ][ 1 ].render();
	mIntroSquares[ 0 ][ 2 ].render();
	mIntroSquares[ 0 ][ 5 ].render();
	mIntroSquares[ 0 ][ 6 ].render();

	mIntroSquares[ 2 ][ 1 ].render();
	mIntroSquares[ 2 ][ 2 ].render();
	mIntroSquares[ 2 ][ 5 ].render();
	mIntroSquares[ 2 ][ 6 ].render();

	break;

      case TETROMINO_S:
	mIntroSquares[ 1 ][ 1 ].fill( TETROMINO_S );
	mIntroSquares[ 1 ][ 2 ].fill( TETROMINO_S );
	mIntroSquares[ 1 ][ 3 ].fill( TETROMINO_S );
	mIntroSquares[ 1 ][ 4 ].fill( TETROMINO_S );
	
	mIntroSquares[ 3 ][ 1 ].fill( TETROMINO_S );
	mIntroSquares[ 3 ][ 2 ].fill( TETROMINO_S );
	mIntroSquares[ 3 ][ 3 ].fill( TETROMINO_S );
	mIntroSquares[ 3 ][ 4 ].fill( TETROMINO_S );
	
	mIntroSquares[ 1 ][ 1 ].setAlpha( alpha );
	mIntroSquares[ 1 ][ 2 ].setAlpha( alpha );
	mIntroSquares[ 1 ][ 3 ].setAlpha( alpha );
	mIntroSquares[ 1 ][ 4 ].setAlpha( alpha );

	mIntroSquares[ 3 ][ 1 ].setAlpha( alpha );
	mIntroSquares[ 3 ][ 2 ].setAlpha( alpha );
	mIntroSquares[ 3 ][ 3 ].setAlpha( alpha );
	mIntroSquares[ 3 ][ 4 ].setAlpha( alpha );

	mIntroSquares[ 1 ][ 1 ].render();
	mIntroSquares[ 1 ][ 2 ].render();
	mIntroSquares[ 1 ][ 3 ].render();
	mIntroSquares[ 1 ][ 4 ].render();

	mIntroSquares[ 3 ][ 1 ].render();
	mIntroSquares[ 3 ][ 2 ].render();
	mIntroSquares[ 3 ][ 3 ].render();
	mIntroSquares[ 3 ][ 4 ].render();

	break;

      case TETROMINO_T:
	mIntroSquares[ 1 ][ 1 ].fill( TETROMINO_T );
	mIntroSquares[ 1 ][ 3 ].fill( TETROMINO_T );
	mIntroSquares[ 1 ][ 4 ].fill( TETROMINO_T );
	mIntroSquares[ 1 ][ 5 ].fill( TETROMINO_T );

	mIntroSquares[ 3 ][ 1 ].fill( TETROMINO_T );
	mIntroSquares[ 3 ][ 3 ].fill( TETROMINO_T );
	mIntroSquares[ 3 ][ 4 ].fill( TETROMINO_T );
	mIntroSquares[ 3 ][ 5 ].fill( TETROMINO_T );

	mIntroSquares[ 1 ][ 1 ].setAlpha( alpha );
	mIntroSquares[ 1 ][ 3 ].setAlpha( alpha );
	mIntroSquares[ 1 ][ 4 ].setAlpha( alpha );
	mIntroSquares[ 1 ][ 5 ].setAlpha( alpha );
	
	mIntroSquares[ 3 ][ 1 ].setAlpha( alpha );
	mIntroSquares[ 3 ][ 3 ].setAlpha( alpha );
	mIntroSquares[ 3 ][ 4 ].setAlpha( alpha );
	mIntroSquares[ 3 ][ 5 ].setAlpha( alpha );
	
	mIntroSquares[ 1 ][ 1 ].render();
	mIntroSquares[ 1 ][ 3 ].render();
	mIntroSquares[ 1 ][ 4 ].render();
	mIntroSquares[ 1 ][ 5 ].render();
	
	mIntroSquares[ 3 ][ 1 ].render();
	mIntroSquares[ 3 ][ 3 ].render();
	mIntroSquares[ 3 ][ 4 ].render();
	mIntroSquares[ 3 ][ 5 ].render();

	break; 

      case TETROMINO_Z:
	mIntroSquares[ 1 ][ 0 ].fill( TETROMINO_Z );
	mIntroSquares[ 1 ][ 1 ].fill( TETROMINO_Z );
	mIntroSquares[ 1 ][ 4 ].fill( TETROMINO_Z );
	mIntroSquares[ 1 ][ 5 ].fill( TETROMINO_Z );
	  
	mIntroSquares[ 3 ][ 0 ].fill( TETROMINO_Z );
	mIntroSquares[ 3 ][ 1 ].fill( TETROMINO_Z );
	mIntroSquares[ 3 ][ 4 ].fill( TETROMINO_Z );
	mIntroSquares[ 3 ][ 5 ].fill( TETROMINO_Z );
	
	mIntroSquares[ 1 ][ 0 ].setAlpha( alpha );
	mIntroSquares[ 1 ][ 1 ].setAlpha( alpha );
	mIntroSquares[ 1 ][ 4 ].setAlpha( alpha );
	mIntroSquares[ 1 ][ 5 ].setAlpha( alpha );
	
	mIntroSquares[ 3 ][ 0 ].setAlpha( alpha );
	mIntroSquares[ 3 ][ 1 ].setAlpha( alpha );
	mIntroSquares[ 3 ][ 4 ].setAlpha( alpha );
	mIntroSquares[ 3 ][ 5 ].setAlpha( alpha );
	
	mIntroSquares[ 1 ][ 0 ].render();
	mIntroSquares[ 1 ][ 1 ].render();
	mIntroSquares[ 1 ][ 4 ].render();
	mIntroSquares[ 1 ][ 5 ].render();
	
	mIntroSquares[ 3 ][ 0 ].render();
	mIntroSquares[ 3 ][ 1 ].render();
	mIntroSquares[ 3 ][ 4 ].render();
	mIntroSquares[ 3 ][ 5 ].render();

	break;
    }

    gPressEnterTexture.setAlpha( 255 - ( 255 * ( ( currentTicks % 2000 ) - 1000 ) * ( ( currentTicks % 2000 ) - 1000 ) / 1000000 ) );
    gPressEnterTexture.render( 0, 0 );
  }
  else
  {
    int x = 275, y = 25;
    
    if( currentTicks < 20000 )
    {
      for( int i = 0; i < TOTAL_SCORES; i++ )
      {
	int limit = 1000 * i ;
	if( ( currentTicks - 10000 ) < limit )
	{
	  gListTexture.setAlpha( 0 );
	}
	else if( ( currentTicks - 10000 ) >= limit && ( currentTicks - 10000 ) < ( limit + 1000 ) )
	{
	  alpha = 255 * ( ( currentTicks - 10000 ) - limit ) / 1000;
	  gListTexture.setAlpha( alpha );
	  gListTextTextures[ i ].setAlpha( alpha );
	  gListTextTextures[ i + TOTAL_SCORES ].setAlpha( alpha );
	}
	else if( ( currentTicks - 10000 ) >= ( limit + 1000 ) && ( currentTicks - 10000 ) < 9000 )
	{
	  gListTexture.setAlpha( 255 );
	}
	else
	{
	  gListTexture.setAlpha( 255 - ( 255 * ( ( currentTicks - 10000 ) - 9000 ) / 1000 ) );
	  gListTextTextures[ i ].setAlpha( 255 - ( 255 * ( ( currentTicks - 10000 ) - 9000 ) / 1000 ) );
	  gListTextTextures[ i + TOTAL_SCORES ].setAlpha( 255 - ( 255 * ( ( currentTicks - 10000 ) - 9000 ) / 1000 ) );
	}
	gListTexture.render( x, y + ( i * 100 ), &gListClips[ i ] );
	gListTextTextures[ i ].render( mListPositions[ i ].x, mListPositions[ i ].y );
	gListTextTextures[ i + TOTAL_SCORES ].render( mListCenters[ i ].x + 13, mListCenters[ i ].y + 13 );
      }
    }
  }
}
