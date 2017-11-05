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
#include "../functions/functions.h"
#include "GameOver.h"

GameOver::GameOver( Stats* stats, Square* gridSquares, SDL_Rect& yourScoreArea )
{
  mGridSquares = gridSquares;

  randomPermutation( mSquareSequence, TOTAL_SQUARES );

  mLastBG = stats->currentBG;

  SDL_Color yourScoreColor = { 233, 82, 82 };
  std::string yourScore = std::to_string( stats->score );
  gYourScoreTextTexture.loadFromRenderedText( yourScore.c_str(), yourScoreColor );

  SDL_Point yourScoreCenter;
  yourScoreCenter.x = yourScoreArea.x + ( yourScoreArea.w / 2 );
  yourScoreCenter.y = yourScoreArea.y + ( yourScoreArea.h / 2 );
  mYourScorePosition.x = yourScoreCenter.x - ( gYourScoreTextTexture.getWidth() / 2 );
  mYourScorePosition.y = yourScoreCenter.y - ( gYourScoreTextTexture.getHeight() / 2 );

  mNextState = GAME_STATE_NULL;

  mTimer.start();

  Mix_PlayChannel( MIX_CHANNEL_GAMEOVER, gGameOverSound, 0 );
}

GameOver::~GameOver()
{
  mGridSquares = NULL;
}

void GameOver::handleEvent( SDL_Event& e )
{

}

void GameOver::logic()
{
  if( mTimer.getTicks() >= 7000 )
  {
    mNextState = GAME_STATE_SCORELIST;
  }

  if( Mix_Playing( -1 ) > 0 )
  {
    Mix_FadeOutChannel( -1, 7000 );
  }
}

void GameOver::render()
{
  int currentTicks = mTimer.getTicks();

  if( currentTicks < 4000 )
  {
    gBGTextures[ mLastBG ].setAlpha( 255 - ( 255 * currentTicks / 4000 ) );
    gBGTextures[ mLastBG ].render( 0, 0 );
    gPlayBGTexture.setAlpha( 255 - ( 255 * currentTicks / 4000 ) );
    gPlayBGTexture.render( 0, 0 );

    mGridSquares[ mSquareSequence[ currentTicks / 20 ] ].clear();
    
    for( int i = ( 2 * TOTAL_COLS ); i < TOTAL_SQUARES; i++ )
    {
      if( mGridSquares[ i ].getState() != SQUARE_STATE_BLANK )
      {
	mGridSquares[ i ].render();
      }
    }
  }

  gBlankBGTexture.render( 0, 0 );
  
  if( currentTicks < 3000 || currentTicks >= 7000 )
  {
    gGameOverTexture.setAlpha( 0 );
    gYourScoreTextTexture.setAlpha( 0 );
  }
  else if( currentTicks >= 3000 && currentTicks < 4000 )
  {
    gGameOverTexture.setAlpha( 255 * ( currentTicks - 3000 ) / 1000 );
    gYourScoreTextTexture.setAlpha( 255 * ( currentTicks - 3000 ) / 1000 );
  }
  else if( currentTicks >= 6000 )
  {
    gGameOverTexture.setAlpha( 255 - ( 255 * ( currentTicks - 6000 ) / 1000 ) );
    gYourScoreTextTexture.setAlpha( 255 - ( 255 * ( currentTicks - 6000 ) / 1000 ) );
  }
  else
  {
    gGameOverTexture.setAlpha( 255 );
    gYourScoreTextTexture.setAlpha( 255 );
  }

  gGameOverTexture.render( 0, 0 );
  gYourScoreTextTexture.render( mYourScorePosition.x, mYourScorePosition.y );
}
