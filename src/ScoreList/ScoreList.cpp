#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <string>

#include "../constants.h"
#include "../globals/globals.h"
#include "../LTexture/LTexture.h"
#include "../textures/textures.h"
#include "../Timer/Timer.h"
#include "../GameState/GameState.h"
#include "ScoreList.h"

ScoreList::ScoreList( int score, SDL_Rect& listArea )
{
  mNewScore = score;
  mName = "";
  mNameChanged = false;

  for( int i = 0; i < TOTAL_SCORES; i++ )
  {
    mListCenters[ i ].x = listArea.x + ( listArea.w / 2 );
    mListCenters[ i ].y = listArea.y + ( i * gListClips[ 0 ].h ) + ( gListClips[ 0 ].h / 2 );
  }

  mNewRank = TOTAL_SCORES;

  mGotHighScore = false;

  for( int i = TOTAL_SCORES - 1; i >= 0 && mNewScore > gScores[ i ].score; i-- )
  {
    mGotHighScore = true;
    mNewRank = i;
  }

  if( mGotHighScore )
  {
    for( int i = TOTAL_SCORES - 1; i > mNewRank; i-- )
    {
      strcpy( gScores[ i ].name, gScores[ i - 1 ].name );
      gScores[ i ].score = gScores[ i - 1 ].score;
    }

    gScores[ mNewRank ].score = mNewScore;
    
    SDL_Color scoreColor = { 0, 0, 0 };

    std::string score = std::to_string( mNewScore );
    gNewScoreTextTextures[ 1 ].loadFromRenderedText( score.c_str(), scoreColor ); 

    gListTexture.setAlpha( 255 );
  }
  else
  {
    for( int i = 0; i < TOTAL_SCORES; i++ )
    {
      std::string name = gScores[ i ].name;
   
      if( name == "" )
      {
	name = " ";
      }

      SDL_Color nameColor = { 255, 255, 255 };
      SDL_Color scoreColor = { 0, 0, 0 };
      
      gListTextTextures[ i ].loadFromRenderedText( name, nameColor );
      gListTextTextures[ i ].setAlpha( 0 );
      std::string score = std::to_string( gScores[ i ].score );
      gListTextTextures[ i + TOTAL_SCORES ].loadFromRenderedText( score.c_str(), scoreColor );
      gListTextTextures[ i + TOTAL_SCORES ].setAlpha( 0 );
      mListPositions[ i ].x = mListCenters[ i ].x - ( gListTextTextures[ i ].getWidth() / 2 );
      mListPositions[ i ].y = mListCenters[ i ].y - ( gListTextTextures[ i ].getHeight() / 2 );
    }
  }

  for( int i = 0; i < TOTAL_SCORES; i++ )
  {
    gListTextTextures[ i ].setAlpha( 0 );
    gListTextTextures[ i + TOTAL_SCORES ].setAlpha( 0 );
  }

  mTimer.start();
}

ScoreList::~ScoreList()
{

}

void ScoreList::handleEvent( SDL_Event& e )
{
  if( mGotHighScore )
  {
    if( e.type == SDL_KEYDOWN )
    {
      if( e.key.keysym.sym == SDLK_RETURN )
      {
	if( mGotHighScore && mName.length() > 0 )
	{
	  mGotHighScore = false;

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
	    mListPositions[ i ].x = mListCenters[ i ].x - ( gListTextTextures[ i ].getWidth() / 2 );
	    mListPositions[ i ].y = mListCenters[ i ].y - ( gListTextTextures[ i ].getHeight() / 2 );
	  }

	  mTimer.start();
	}
      }
      else if( e.key.keysym.sym == SDLK_BACKSPACE )
      {
	if( mGotHighScore && mName.length() > 0 )
	{
	  mName.pop_back();
	  mNameChanged = true;
	}
      }
    }
    else if( e.type == SDL_TEXTINPUT )
    {
      if( mName.length() < 10 )
      {
	mName += e.text.text;
	mNameChanged = true;
      }
    }
  }
}

void ScoreList::logic()
{
  if( mGotHighScore )
  {
    strcpy( gScores[ mNewRank ].name, mName.c_str() );
  }
  else
  {
    int currentTicks = mTimer.getTicks();

    if( currentTicks >= 8000 )
    {
      if( Mix_Playing( MIX_CHANNEL_MUSIC ) == 1 )
      {
	Mix_FadeOutChannel( MIX_CHANNEL_MUSIC, 2000 );
      }
    }

    if( currentTicks >= 10000 )
    {
      mNextState = GAME_STATE_INTRO;
    }
  }

  if( Mix_Playing( MIX_CHANNEL_MUSIC ) == 0 )
  {
    Mix_PlayChannel( MIX_CHANNEL_MUSIC, gScoreMusic, -1 );
  }
}

void ScoreList::render()
{
  int currentTicks = mTimer.getTicks();

  SDL_Color nameColor = { 255, 255, 255 };
  SDL_Color scoreColor = { 0, 0, 0 };

  gBlankBGTexture.render( 0, 0 );

  // If the player is entering a high score
  if( mGotHighScore )
  {
    gEnterNameTexture.setAlpha( 255 - ( 255 * ( ( currentTicks % 2000 ) - 1000 ) * ( ( currentTicks % 2000 ) - 1000 ) / 1000000 ) );
    gEnterNameTexture.render( 0, 0 );

    gListTexture.render( 275, 225, &gListClips[ mNewRank ] );

    if( mNameChanged )
    {
      mNameChanged = false;

      if( mName != "" )
      {
	gNewScoreTextTextures[ 0 ].loadFromRenderedText( mName.c_str(), nameColor );
      }
      else
      {
	gNewScoreTextTextures[ 0 ].loadFromRenderedText( " ", nameColor );
      }

      mNewScorePosition.x = mListCenters[ TOTAL_SCORES / 2 ].x - ( gNewScoreTextTextures[ 0 ].getWidth() / 2 );
      mNewScorePosition.y = mListCenters[ TOTAL_SCORES / 2 ].y - ( gNewScoreTextTextures[ 0 ].getHeight() / 2 );
    }

    gNewScoreTextTextures[ 0 ].render( mNewScorePosition.x, mNewScorePosition.y );
    gNewScoreTextTextures[ 1 ].render( mListCenters[ TOTAL_SCORES / 2 ].x + 13, mListCenters[ TOTAL_SCORES / 2 ].y + 13 );
  }
  // If the player is not entering a high score
  else
  {
    int x = 275, y = 25;
    
    for( int i = 0; i < TOTAL_SCORES; i++ )
    {
      int limit = 1000 * i;
      if( currentTicks < limit )
      {
	gListTexture.setAlpha( 0 );
      }
      else if( currentTicks >= limit && currentTicks < ( limit + 1000 ) )
      {
	int a = 255 * ( currentTicks - limit ) / 1000;
	gListTexture.setAlpha( a );
	gListTextTextures[ i ].setAlpha( a );
	gListTextTextures[ i + TOTAL_SCORES ].setAlpha( a );
      }
      else if( currentTicks >= ( limit + 1000 ) && currentTicks < 9000 )
      {
	gListTexture.setAlpha( 255 );
      }
      else
      {
	gListTexture.setAlpha( 255 - ( 255 * ( currentTicks - 9000 ) / 1000 ) );
	gListTextTextures[ i ].setAlpha( 255 - ( 255 * ( currentTicks - 9000 ) / 1000 ) );
	gListTextTextures[ i + TOTAL_SCORES ].setAlpha( 255 - ( 255 * ( currentTicks - 9000 ) / 1000 ) );
      }
      gListTexture.render( x, y + ( i * 100 ), &gListClips[ i ] );
      gListTextTextures[ i ].render( mListPositions[ i ].x, mListPositions[ i ].y );
      gListTextTextures[ i + TOTAL_SCORES ].render( mListCenters[ i ].x + 13, mListCenters[ i ].y + 13 );
    }
  }
}
