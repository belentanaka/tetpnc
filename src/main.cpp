#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <time.h>

#include "constants.h"
#include "globals/globals.h"
#include "LTexture/LTexture.h"
#include "textures/textures.h"
#include "Square/Square.h"
#include "GameState/GameState.h"
#include "Intro/Intro.h"
#include "Play/Play.h"
#include "GameOver/GameOver.h"
#include "ScoreList/ScoreList.h"

bool init()
{
  bool success = true;
  
  if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
  {
    printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
    success = false;
  }
  else
  {
    gWindow = SDL_CreateWindow( "TETPNC", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
    if( gWindow == NULL )
    {
      printf( "SDL could not create window! SDL Error: %s\n", SDL_GetError() );
      success = false;
    }
    else
    {
      gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
      if( gRenderer == NULL )
      {
	printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
	success = false;
      }
      else
      {
	SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
	
	int imgFlags = IMG_INIT_PNG;
	if( !( IMG_Init( imgFlags ) & imgFlags ) )
	{
	  printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
	  success = false;
	}
	else
	{
	  if( TTF_Init() == -1 )
	  {
	    printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
	    success = false;
	  }
	  else
	  {
	    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 4096 ) < 0 )
	    {
	      printf( "SDL_mixer could not initializae! SDL_mixer Error: %s\n", Mix_GetError() );
	      success = false;
	    }
	  }
	}
      }
    }
  }

  return success;
}

bool loadMedia()
{
  bool success = true;

  SDL_RWops* file = SDL_RWFromFile( "bin/scores.bin", "r+b" );
  if( file == NULL )
  {
    file = SDL_RWFromFile( "bin/scores.bin", "w+b" );
    if( file != NULL )
    {
      for( int i = 0; i < TOTAL_SCORES; i++ )
      {
	std::string name = "TAKASHI";
	strcpy( gScores[ i ].name, name.c_str() );
	gScores[ i ].score = 500 - ( i * 100 );
	SDL_RWwrite( file, &gScores[ i ], sizeof( Score ), 1 );
      }

      SDL_RWclose( file );
    }
    else
    {
      printf( "Error: Unable to create file! SDL Error: %s\n", SDL_GetError() );
      success = false;
    }
  }
  else
  {
    for( int i = 0; i < TOTAL_SCORES; i++ )
    {
      SDL_RWread( file, &gScores[ i ], sizeof( Score ), 1 );
    }

    SDL_RWclose( file );
  }

  if( !gBlankBGTexture.loadFromFile( "images/blank_bg.png" ) )
  {
    printf( "Failed to load blank background texture!\n" );
    success = false;
  }

  if( !gPressEnterTexture.loadFromFile( "images/press_enter.png" ) )
  {
    printf( "Failed to load press enter prompt texture!\n" );
    success = false;
  }

  if( !gPlayBGTexture.loadFromFile( "images/play_bg.png" ) )
  {
    printf( "Failed to load play background texture!\n" );
    success = false;
  }

  if( !gPausedTexture.loadFromFile( "images/paused.png" ) )
  {
    printf( "Failed to load pause prompt texture!\n" );
    success = false;
  }

  if( !gGameOverTexture.loadFromFile( "images/game_over.png" ) )
  {
    printf( "Failed to load game over prompt texture!\n" );
    success = false;
  }

  if( !gEnterNameTexture.loadFromFile( "images/enter_name.png" ) )
  {
    printf( "Failed to load enter name prompt texture!\n" );
    success = false;
  }

  if( !gBlackTexture.loadFromFile( "images/black.png" ) )
  {
    printf( "Failed to load black texture!\n" );
    success = false;
  }

  if( !gTetrisTexture.loadFromFile( "images/tetpnc.png" ) )
  {
    printf( "Failed to load tetpnc texture!\n" );
    success = false;
  }

  if( !gListTexture.loadFromFile( "images/scores.png" ) )
  {
    printf( "Failed to load score list texture!\n" );
    success = false;
  }
  else
  {
    gListTexture.setBlendMode( SDL_BLENDMODE_BLEND );

    for( int i = 0; i < TOTAL_SCORES; i++ )
    {
      gListClips[ i ].x = 0;
      gListClips[ i ].y = i * 100;
      gListClips[ i ].w = 300;
      gListClips[ i ].h = 100;
    }
  }

  if( !gSquareSpriteTexture.loadFromFile( "images/blocks.png" ) )
  {
    printf( "Failed to load block sprite sheet texture!\n" );
    success = false;
  }
  else
  {
    gSquareSpriteTexture.setBlendMode( SDL_BLENDMODE_BLEND );

    for( int i = 0; i < SQUARE_SPRITE_TOTAL; i++ )
    {
      gSquareSpriteClips[ i ].x = i * Square::SQUARE_WIDTH;
      gSquareSpriteClips[ i ].y = 0;
      gSquareSpriteClips[ i ].w = Square::SQUARE_WIDTH;
      gSquareSpriteClips[ i ].h = Square::SQUARE_HEIGHT;
    }
  }

  if( !gHandBlackTexture.loadFromFile( "images/hands1.png" ) )
  {
    printf( "Failed to load black hand sheet texture!\n" );
    success = false;
  }
  else
  {
    gHandBlackTexture.setBlendMode( SDL_BLENDMODE_BLEND );
  }

  if( !gHandWhiteTexture.loadFromFile( "images/hands2.png" ) )
  {
    printf( "Failed to load white hand sheet texture!\n" );
    success = false;
  }
  else
  {
    gHandWhiteTexture.setBlendMode( SDL_BLENDMODE_BLEND );

    for( int i = 0; i < 5; i++ )
    {
      gHandClips[ i ].x = 0;
      gHandClips[ i ].y = i * 275;
      gHandClips[ i ].w = 850;
      gHandClips[ i ].h = 275;
    }
  }

  for( int i = 0; i < TOTAL_BG; i++ )
  {
    std::string name = "images/bg" + std::to_string( i + 1 ) + ".png";
    if( !gBGTextures[ i ].loadFromFile( name ) )
    {
      printf( "Failed to load background %d texture!\n", i + 1 );
      success = false;
    }
  }

  gFont = TTF_OpenFont( "fonts/Krungthep.ttf", 25 );
  if( gFont == NULL )
  {
    printf( "Failed to load Krungthep font! SDL_ttf Error: %s\n", TTF_GetError() );
    success = false;
  }

  gStartMusic = Mix_LoadWAV( "music/start.wav" );
  if( gStartMusic == NULL )
  {
    printf( "Failed to load start music! SDL_mixer Error: %s\n", Mix_GetError() );
    success = false;
  }

  gScoreMusic = Mix_LoadWAV( "music/score.wav" );
  if( gScoreMusic == NULL )
  {
    printf( "Failed to load score music! SDL_mixer Error: %s\n", Mix_GetError() );
    success = false;
  }

  gMoveSound = Mix_LoadWAV( "sounds/move.wav" );
  if( gMoveSound == NULL )
  {
    printf( "Failed to load move sound! SDL_mixer Error: %s\n", Mix_GetError() );
    success = false;
  }

  gHoldSound = Mix_LoadWAV( "sounds/hold.wav" );
  if( gHoldSound == NULL )
  {
    printf( "Failed to load hold sound! SDL_mixer Error: %s\n", Mix_GetError() );
    success = false;
  }

  gLandSound = Mix_LoadWAV( "sounds/land.wav" );
  if( gLandSound == NULL )
  {
    printf( "Failed to load land sound! SDL_Mixer Error: %s\n", Mix_GetError() );
    success = false;
  }

  gClearSound = Mix_LoadWAV( "sounds/clear.wav" );
  if( gClearSound == NULL )
  {
    printf( "Failed to load clear sound! SDL_mixer Error: %s\n", Mix_GetError() );
    success = false;
  }

  gTetrisSound = Mix_LoadWAV( "sounds/tetris.wav" );
  if( gTetrisSound == NULL )
  {
    printf( "Failed to load tetris sound! SDL_mixer Error: %s\n", Mix_GetError() );
    success = false;
  }

  gGameOverSound = Mix_LoadWAV( "sounds/gameover.wav" );
  if( gGameOverSound == NULL )
  {
    printf( "Failed to load game over sound! SDL_mixer Error: %s\n", Mix_GetError() );
    success = false;
  }

  for( int i = 0; i < TOTAL_BGM; i++ )
  {
    gBGMusic[ i ] = NULL;
    std::string name = "music/bgm" + std::to_string( i + 1 ) + ".wav";
    gBGMusic[ i ] = Mix_LoadWAV( name.c_str() );
    if( gBGMusic[ i ] == NULL )
    {
      printf( "Failed to load background music %d! SDL_mixer Error: %s\n", i + 1, Mix_GetError() );
      success = false;
    }
  }

  return success;
}

void close()
{
  SDL_RWops* file = SDL_RWFromFile( "bin/scores.bin", "w+b" );
  if( file != NULL )
  {
    for( int i = 0; i < TOTAL_SCORES; i++ )
    {
      if( gScores[ i ].name == "" )
      {
	strcpy( gScores[ i ].name, " " );
      }
      SDL_RWwrite( file, &gScores[ i ], sizeof( Score ), 1 );
    }
    
    SDL_RWclose( file );
  }
  else
  {
    printf( "Error: Could not save file! SDL Error: %s\n", SDL_GetError() );
  }

  gBlankBGTexture.free();
  gPressEnterTexture.free();
  gPlayBGTexture.free();
  gPausedTexture.free();
  gGameOverTexture.free();
  gEnterNameTexture.free();
  gListTexture.free();
  gBlackTexture.free();
  gSquareSpriteTexture.free();
  gHandBlackTexture.free();
  gHandWhiteTexture.free();
  gScoreTextTexture.free();
  gLinesTextTexture.free();
  gLevelTextTexture.free();
  gYourScoreTextTexture.free();

  for( int i = 0; i < TOTAL_BG; i++ )
  {
    gBGTextures[ i ].free();
  }

  for( int i = 0; i < ( 2 * TOTAL_SCORES ); i++ )
  {
    gListTextTextures[ i ].free();
  }

  gNewScoreTextTextures[ 0 ].free();
  gNewScoreTextTextures[ 1 ].free();

  TTF_CloseFont( gFont );
  gFont = NULL;

  Mix_FreeChunk( gStartMusic );
  gStartMusic = NULL;
  Mix_FreeChunk( gScoreMusic );
  gScoreMusic = NULL;
  Mix_FreeChunk( gMoveSound );
  gMoveSound = NULL;
  Mix_FreeChunk( gHoldSound );
  gHoldSound = NULL;
  Mix_FreeChunk( gLandSound );
  gLandSound = NULL;
  Mix_FreeChunk( gClearSound );
  gClearSound = NULL;
  Mix_FreeChunk( gTetrisSound );
  gTetrisSound = NULL;

  for( int i = 0; i < TOTAL_BGM; i++ )
  {
    Mix_FreeChunk( gBGMusic[ i ] );
    gBGMusic[ i ] = NULL;
  }

  SDL_DestroyRenderer( gRenderer );
  SDL_DestroyWindow( gWindow );
  gWindow = NULL;
  gRenderer = NULL;

  Mix_Quit();
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}

int main()
{
  if( !init() )
  {
    printf( "Failed to initialize!\n" );
  }
  else 
  {
    if( !loadMedia() )
    {
      printf( "Failed to load media!\n" );
    }   
    else
    {
      bool quit = false;
      SDL_Event e;

      SDL_Rect gridArea;
      gridArea.x = 300;
      gridArea.y = 0;
      gridArea.w = 250;
      gridArea.h = 450;

      SDL_Rect nextAreaTop;
      nextAreaTop.x = 625;
      nextAreaTop.y = 125;
      nextAreaTop.w = 125;
      nextAreaTop.h = 100;

      SDL_Rect nextAreaMid;
      nextAreaMid.x = 625;
      nextAreaMid.y = 225;
      nextAreaMid.w = 125;
      nextAreaMid.h = 100;

      SDL_Rect nextAreaBot;
      nextAreaBot.x = 625;
      nextAreaBot.y = 325;
      nextAreaBot.w = 125;
      nextAreaBot.h = 100;

      SDL_Rect holdArea;
      holdArea.x = 100;
      holdArea.y = 125;
      holdArea.w = 125;
      holdArea.h = 100;

      SDL_Rect startAreaL;
      startAreaL.x = 125;
      startAreaL.y = 260;
      startAreaL.w = 100;
      startAreaL.h = 75;

      SDL_Rect startAreaR;
      startAreaR.x = 625;
      startAreaR.y = 260;
      startAreaR.w = 100;
      startAreaR.h = 75;

      SDL_Rect scoreArea;
      scoreArea.x = 75;
      scoreArea.y = 300;
      scoreArea.w = 150;
      scoreArea.h = 50;

      SDL_Rect linesArea;
      linesArea.x = 75;
      linesArea.y = 375;
      linesArea.w = 150;
      linesArea.h = 50;

      SDL_Rect levelArea;
      levelArea.x = 75;
      levelArea.y = 450;
      levelArea.w = 150;
      levelArea.h = 50;

      SDL_Rect yourScoreArea;
      yourScoreArea.x = 280;
      yourScoreArea.y = 300;
      yourScoreArea.w = 290;
      yourScoreArea.h = 50;

      SDL_Rect listArea;
      listArea.x = ( SCREEN_WIDTH / 2 ) - 150;
      listArea.y = ( SCREEN_HEIGHT / 2 ) - 250;
      listArea.w = 300;
      listArea.h = 500;

      Square gridSquares[ TOTAL_SQUARES ];
      for( int i = 0; i < TOTAL_SQUARES; i++ )
      {
	int x = ( ( i % TOTAL_COLS ) * Square::SQUARE_WIDTH ) + gridArea.x;
	int y = ( ( i / TOTAL_COLS ) * Square::SQUARE_HEIGHT ) + gridArea.y;
	gridSquares[ i ].setPosition( x, y );
      }

      Square nextSquaresTop1[ 8 ];
      for( int i = 0; i < 8; i++ )
      {
	int x = ( ( i % 4 ) * Square::SQUARE_WIDTH ) + nextAreaTop.x + ( nextAreaTop.w / 2 ) - ( Square::SQUARE_WIDTH * 2 );
	int y = ( ( i / 4 ) * Square::SQUARE_HEIGHT ) + nextAreaTop.y + ( nextAreaTop.h / 2 ) - ( Square::SQUARE_HEIGHT * 2 );
	nextSquaresTop1[ i ].setPosition( x, y );
      }

      Square nextSquaresTop2[ 6 ];
      for( int i = 0; i < 6; i++ )
      {
	int x = ( ( i % 3 ) * Square::SQUARE_WIDTH ) + nextAreaTop.x + ( nextAreaTop.w / 2 ) - ( Square::SQUARE_WIDTH * 3 / 2 );
	int y = ( ( i / 3 ) * Square::SQUARE_HEIGHT ) + nextAreaTop.y + ( nextAreaTop.h / 2 ) - ( Square::SQUARE_HEIGHT * 2 );
	nextSquaresTop2[ i ].setPosition( x, y );
      }

      Square nextSquaresMid1[ 8 ];
      for( int i = 0; i < 8; i++ )
      {
	int x = ( ( i % 4 ) * Square::SQUARE_WIDTH ) + nextAreaMid.x + ( nextAreaMid.w / 2 ) - ( Square::SQUARE_WIDTH * 2 );
	int y = ( ( i / 4 ) * Square::SQUARE_HEIGHT ) + nextAreaMid.y + ( nextAreaMid.h / 2 ) - ( Square::SQUARE_HEIGHT * 2 );
	nextSquaresMid1[ i ].setPosition( x, y );
      }

      Square nextSquaresMid2[ 6 ];
      for( int i = 0; i < 6; i++ )
      {
	int x = ( ( i % 3 ) * Square::SQUARE_WIDTH ) + nextAreaMid.x + ( nextAreaMid.w / 2 ) - ( Square::SQUARE_WIDTH * 3 / 2 );
	int y = ( ( i / 3 ) * Square::SQUARE_HEIGHT ) + nextAreaMid.y + ( nextAreaMid.h / 2 ) - ( Square::SQUARE_HEIGHT * 2 );
	nextSquaresMid2[ i ].setPosition( x, y );
      }

      Square nextSquaresBot1[ 8 ];
      for( int i = 0; i < 8; i++ )
      {
	int x = ( ( i % 4 ) * Square::SQUARE_WIDTH ) + nextAreaBot.x + ( nextAreaBot.w / 2 ) - ( Square::SQUARE_WIDTH * 2 );
	int y = ( ( i / 4 ) * Square::SQUARE_HEIGHT ) + nextAreaBot.y + ( nextAreaBot.h / 2 ) - ( Square::SQUARE_HEIGHT * 2 );
	nextSquaresBot1[ i ].setPosition( x, y );
      }

      Square nextSquaresBot2[ 6 ];
      for( int i = 0; i < 6; i++ )
      {
	int x = ( ( i % 3 ) * Square::SQUARE_WIDTH ) + nextAreaBot.x + ( nextAreaBot.w / 2 ) - ( Square::SQUARE_WIDTH * 3 / 2 );
	int y = ( ( i / 3 ) * Square::SQUARE_HEIGHT ) + nextAreaBot.y + ( nextAreaBot.h / 2 ) - ( Square::SQUARE_HEIGHT * 2 );
	nextSquaresBot2[ i ].setPosition( x, y );
      }

      Square* nextSquares[ 6 ];
      nextSquares[ 0 ] = nextSquaresTop1;
      nextSquares[ 1 ] = nextSquaresTop2;
      nextSquares[ 2 ] = nextSquaresMid1;
      nextSquares[ 3 ] = nextSquaresMid2;
      nextSquares[ 4 ] = nextSquaresBot1;
      nextSquares[ 5 ] = nextSquaresBot2;

      Square holdSquares1[ 8 ];
      for( int i = 0; i < 8; i++ )
      {
	int x = ( ( i % 4 ) * Square::SQUARE_WIDTH ) + holdArea.x + ( holdArea.w / 2 ) - ( Square::SQUARE_WIDTH * 2 );
	int y = ( ( i / 4 ) * Square::SQUARE_HEIGHT ) + holdArea.y + ( holdArea.h / 2 ) - ( Square::SQUARE_HEIGHT * 2 );
	holdSquares1[ i ].setPosition( x, y );
      }

      Square holdSquares2[ 6 ];
      for( int i = 0; i < 6; i++ )
      {
	int x = ( ( i % 3 ) * Square::SQUARE_WIDTH ) + holdArea.x + ( holdArea.w / 2 ) - ( Square::SQUARE_WIDTH * 3 / 2 );
	int y = ( ( i / 3 ) * Square::SQUARE_HEIGHT ) + holdArea.y + ( holdArea.h / 2 ) - ( Square::SQUARE_HEIGHT * 2 );
	holdSquares2[ i ].setPosition( x, y );
      }

      Square* holdSquares[ 2 ];
      holdSquares[ 0 ] = holdSquares1;
      holdSquares[ 1 ] = holdSquares2;

      Square startSquares1[ 8 ];
      for( int i = 0; i < 8; i++ )
      {
	int x = ( ( i % 4 ) * Square::SQUARE_WIDTH ) + startAreaL.x + ( startAreaL.w / 2 ) - ( Square::SQUARE_WIDTH * 2 );
	int y = ( ( i / 4 ) * Square::SQUARE_HEIGHT ) + startAreaL.y + ( startAreaL.h / 2 ) - ( Square::SQUARE_HEIGHT * 2 );
	startSquares1[ i ].setPosition( x, y );
      }

      Square startSquares2[ 6 ];
      for( int i = 0; i < 6; i++ )
      {
	int x = ( ( i % 3 ) * Square::SQUARE_WIDTH ) + startAreaL.x + ( startAreaL.w / 2 ) - ( Square::SQUARE_WIDTH * 3 / 2 );
	int y = ( ( i / 3 ) * Square::SQUARE_HEIGHT ) + startAreaL.y + ( startAreaL.h / 2 ) - ( Square::SQUARE_HEIGHT * 2 );
	startSquares2[ i ].setPosition( x, y );
      }

      Square startSquares3[ 8 ];
      for( int i = 0; i < 8; i++ )
      {
	int x = ( ( i % 4 ) * Square::SQUARE_WIDTH ) + startAreaR.x + ( startAreaR.w / 2 ) - ( Square::SQUARE_WIDTH * 2 );
	int y = ( ( i / 4 ) * Square::SQUARE_HEIGHT ) + startAreaR.y + ( startAreaR.h / 2 ) - ( Square::SQUARE_HEIGHT * 2 );
	startSquares3[ i ].setPosition( x, y );
      }

      Square startSquares4[ 6 ];
      for( int i = 0; i < 6; i++ )
      {
	int x = ( ( i % 3 ) * Square::SQUARE_WIDTH ) + startAreaR.x + ( startAreaR.w / 2 ) - ( Square::SQUARE_WIDTH * 3 / 2 );
	int y = ( ( i / 3 ) * Square::SQUARE_HEIGHT ) + startAreaR.y + ( startAreaR.h / 2 ) - ( Square::SQUARE_HEIGHT * 2 );
	startSquares4[ i ].setPosition( x, y );
      }

      Square* startSquares[ 4 ];
      startSquares[ 0 ] = startSquares1;
      startSquares[ 1 ] = startSquares2;
      startSquares[ 2 ] = startSquares3;
      startSquares[ 3 ] = startSquares4;

      SDL_Rect textAreas[ 3 ];
      textAreas[ 0 ] = scoreArea;
      textAreas[ 1 ] = linesArea;
      textAreas[ 2 ] = levelArea;

      Stats stats;

      GameState* g = new Intro( startSquares, listArea );

      srand( time( NULL ) );
      rand();
  
      /*
      const int SCREEN_FPS = 60;
      const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;
      int startTime = SDL_GetTicks();
      int totalTime = 0;
      */

      while( !quit )
      {
	while( SDL_PollEvent( &e ) != 0 )
	{
	  if( e.type == SDL_QUIT )
	  {
	    quit = true;
	  }

	  g->handleEvent( e );
	}

	g->logic();

	SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
	SDL_RenderClear( gRenderer );

	g->render();

	SDL_RenderPresent( gRenderer );

	GameStateFlag nextState = g->getNextState();

	switch( nextState )
	{
	  case GAME_STATE_NULL:
	    break;

	  case GAME_STATE_INTRO:
	    delete g;
	    g = new Intro( startSquares, listArea );
	    break;

	  case GAME_STATE_PLAY:
	    delete g;
	    g = new Play( &stats, gridSquares, nextSquares, holdSquares, textAreas );
	    break;

	  case GAME_STATE_GAMEOVER:
	    delete g;
	    g = new GameOver( &stats, gridSquares, yourScoreArea );
	    break;

	  case GAME_STATE_SCORELIST:
	    delete g;
	    g = new ScoreList( stats.score, listArea );
	    break;

	  case GAME_STATE_ERROR:
	    delete g;
	    printf( "Error found. Exiting game\n" );
	    return 1;
	}
	
	/*
	totalTime = SDL_GetTicks() - startTime;
	if( totalTime < SCREEN_TICKS_PER_FRAME )
	{
	  SDL_Delay( SCREEN_TICKS_PER_FRAME - totalTime );
	}
	*/
      }

      delete g;
    }
  }

  close();
  return 0;
}
