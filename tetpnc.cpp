#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <time.h>

int debug_time_1 = 0;
int debug_time_2 = 0;

const int SCREEN_WIDTH = 850;
const int SCREEN_HEIGHT = 550;

const int TOTAL_ROWS = 20;
const int TOTAL_COLS = 10;
const int TOTAL_SQUARES = TOTAL_ROWS * TOTAL_COLS;

struct Score
{
  char name[ 11 ];
  int score;
};

const int TOTAL_SCORES = 5;
Score gScores[ TOTAL_SCORES ];

enum TetrominoFlag
{
  TETROMINO_I,
  TETROMINO_J,
  TETROMINO_L,
  TETROMINO_O,
  TETROMINO_S,
  TETROMINO_T,
  TETROMINO_Z,
  TETROMINO_COUNT
};

SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font* gFont = NULL;

Mix_Chunk* gStartMusic = NULL;
Mix_Chunk* gScoreMusic = NULL;

Mix_Chunk* gMoveSound = NULL;
Mix_Chunk* gHoldSound = NULL;
Mix_Chunk* gLandSound = NULL;
Mix_Chunk* gClearSound = NULL;
Mix_Chunk* gTetrisSound = NULL;
Mix_Chunk* gGameOverSound = NULL;

const int TOTAL_BGM = 3;
Mix_Chunk* gBGMusic[ TOTAL_BGM ];

class LTexture
{
  public:
  LTexture();
  ~LTexture();
  bool loadFromFile( std::string path );
  bool loadFromRenderedText( std::string textureText, SDL_Color textColor );
  void free();
  void setBlendMode( SDL_BlendMode blending );
  void setAlpha( Uint8 alpha );
  void render( int x, int y, SDL_Rect* clip = NULL );
  int getWidth();
  int getHeight();

  private:
  SDL_Texture* mTexture;
  int mWidth;
  int mHeight;
};

LTexture::LTexture()
{
  mTexture = NULL;
  mWidth = 0;
  mHeight = 0;
}

LTexture::~LTexture()
{
  free();
}

bool LTexture::loadFromFile( std::string path )
{
  free();

  SDL_Texture* newTexture = NULL;

  SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
  if( loadedSurface == NULL )
  {
    printf( "Failed to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
  }
  else
  {
    SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0xFF, 0xFF, 0xFF ) );
    
    newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
    if( newTexture == NULL )
    {
      printf( "Failed to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
    }
    else
    {
      mWidth = loadedSurface->w;
      mHeight = loadedSurface->h;
    }

    SDL_FreeSurface( loadedSurface );
  }

  mTexture = newTexture;
  return mTexture != NULL;
}

bool LTexture::loadFromRenderedText( std::string textureText, SDL_Color textColor )
{
  free();

  SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, textureText.c_str(), textColor );
  if( textSurface == NULL )
  {
    printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
  }
  else
  {
    mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
    if( mTexture == NULL )
    {
      printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
    }
    else
    {
      mWidth = textSurface->w;
      mHeight = textSurface->h;
    }

    SDL_FreeSurface( textSurface );
  }

  return mTexture != NULL;
}
  

void LTexture::free()
{
  if( mTexture != NULL )
  {
    SDL_DestroyTexture( mTexture );
    mTexture = NULL;
    mWidth = 0;
    mHeight = 0;
  }
}

void LTexture::setBlendMode( SDL_BlendMode blending )
{
  SDL_SetTextureBlendMode( mTexture, blending );
}

void LTexture::setAlpha( Uint8 alpha )
{
  SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::render( int x, int y, SDL_Rect* clip )
{
  SDL_Rect renderQuad = { x, y, mWidth, mHeight };

  if( clip != NULL )
  {
    renderQuad.w = clip->w;
    renderQuad.h = clip->h;
  }

  SDL_RenderCopy( gRenderer, mTexture, clip, &renderQuad );
}

int LTexture::getWidth()
{
  return mWidth;
}

int LTexture::getHeight()
{
  return mHeight;
}

LTexture gBlankBGTexture;
LTexture gPressEnterTexture;
LTexture gPlayBGTexture;
LTexture gPausedTexture;
LTexture gGameOverTexture;
LTexture gEnterNameTexture;
LTexture gBlackTexture;
LTexture gTetrisTexture;

const int TOTAL_BGS = 10;
LTexture gBGTextures[ TOTAL_BGS ];

LTexture gHandBlackTexture;
LTexture gHandWhiteTexture;
SDL_Rect gHandClips[ 5 ];

LTexture gListTexture;
SDL_Rect gListClips[ TOTAL_SCORES ];

LTexture gSquareSpriteTexture;

LTexture gScoreTextTexture;
LTexture gLinesTextTexture;
LTexture gLevelTextTexture;
LTexture gListTextTextures[ 2 * TOTAL_SCORES ];
LTexture gNewTextTextures[ 2 ];

enum SquareState
{
  SQUARE_STATE_BLANK,
  SQUARE_STATE_MOVING,
  SQUARE_STATE_STILL,
  SQUARE_STATE_COUNT
};

enum SquareSprite
{
  SQUARE_SPRITE_BLANK,
  SQUARE_SPRITE_I,
  SQUARE_SPRITE_J,
  SQUARE_SPRITE_L,
  SQUARE_SPRITE_O,
  SQUARE_SPRITE_S,
  SQUARE_SPRITE_T,
  SQUARE_SPRITE_Z,
  SQUARE_SPRITE_TOTAL
};

SDL_Rect gSquareSpriteClips[ SQUARE_SPRITE_TOTAL ];

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

Square::Square()
{
  mPosition.x = 0;
  mPosition.y = 0;

  mCurrentState = SQUARE_STATE_BLANK;
  mCurrentSprite = SQUARE_SPRITE_BLANK;
  mAlpha = 255;
}

void Square::setPosition( int x, int y )
{
  mPosition.x = x;
  mPosition.y = y;
}

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

void Square::stop()
{
  mCurrentState = SQUARE_STATE_STILL;
}

void Square::clear()
{
  mCurrentState = SQUARE_STATE_BLANK;
  mCurrentSprite = SQUARE_SPRITE_BLANK;
  mAlpha = 255;
}

void Square::replace( Square& s )
{
  mCurrentState = s.mCurrentState;
  mCurrentSprite = s.mCurrentSprite;
} 

void Square::setAlpha( Uint8 alpha )
{
  mAlpha = alpha;
}

void Square::render()
{
  gSquareSpriteTexture.setAlpha( mAlpha );
  gSquareSpriteTexture.render( mPosition.x, mPosition.y, &gSquareSpriteClips[ mCurrentSprite ] );
}

int Square::getState()
{
  return mCurrentState;
}

class Tetromino
{
  public:
  Tetromino( int type, Square gridSquares[], Uint32 fallDelay );
  ~Tetromino();
  bool fall();
  void handleEvent( SDL_Event& e );
  int getType();

  private:
  void clear();
  void draw();
  void erase();
  void land();
  int mType;
  Square* mGridSquares;
  int mGridPositions[ 4 ];
  int mGhostPositions[ 4 ];
  int mRotation;
  Uint32 mStartTime;
  Uint32 mCurrentTime;
  Uint32 mFallDelay;
  Uint32 mInitialFallDelay;
};

Tetromino::Tetromino( int type, Square gridSquares[], Uint32 fallDelay )
{
  mRotation = 0;
  mStartTime = SDL_GetTicks();
  mCurrentTime = mStartTime;
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

Tetromino::~Tetromino()
{
  clear();
}

bool Tetromino::fall()
{
  bool falling = true;

  mCurrentTime = SDL_GetTicks() - mStartTime;
  if( mCurrentTime >= mFallDelay )
  {
    int a, b, c, d;
    a = mGridPositions[ 0 ] + TOTAL_COLS;
    b = mGridPositions[ 1 ] + TOTAL_COLS;
    c = mGridPositions[ 2 ] + TOTAL_COLS;
    d = mGridPositions[ 3 ] + TOTAL_COLS;
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

    mStartTime = SDL_GetTicks();
  }
   
  return falling;
}

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
      if( mGridPositions[ 0 ] % TOTAL_COLS > 0 &&
	  mGridPositions[ 1 ] % TOTAL_COLS > 0 &&
	  mGridPositions[ 2 ] % TOTAL_COLS > 0 &&
	  mGridPositions[ 3 ] % TOTAL_COLS > 0 &&
	  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
      {
	Mix_PlayChannel( 2, gMoveSound, 0 );

	erase();

	mGridPositions[ 0 ] = a;
	mGridPositions[ 1 ] = b;
	mGridPositions[ 2 ] = c;
	mGridPositions[ 3 ] = d;

	draw();
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
      if( mGridPositions[ 0 ] % TOTAL_COLS < TOTAL_COLS - 1 &&
	  mGridPositions[ 1 ] % TOTAL_COLS < TOTAL_COLS - 1 &&
	  mGridPositions[ 2 ] % TOTAL_COLS < TOTAL_COLS - 1 &&
	  mGridPositions[ 3 ] % TOTAL_COLS < TOTAL_COLS - 1 &&
	  mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
	  mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
	  mGridSquares[ c ].getState() != SQUARE_STATE_STILL &&
	  mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
      {
	Mix_PlayChannel( 2, gMoveSound, 0 );

	erase();

	mGridPositions[ 0 ] = a;
	mGridPositions[ 1 ] = b;
	mGridPositions[ 2 ] = c;
	mGridPositions[ 3 ] = d;
	
	draw();
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
	  b = mGridPositions[ 2 ];
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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    d = c + TOTAL_COLS;

	    if( b / TOTAL_COLS > 0 &&
		b / TOTAL_COLS < TOTAL_ROWS - 2 &&
		mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		mGridSquares[ c ].getState() != SQUARE_STATE_STILL && 
		mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
	    {
	      Mix_PlayChannel( 2, gMoveSound, 0 );
	      
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
	      b = mGridPositions[ 2 ] - TOTAL_COLS - TOTAL_COLS;
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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
		d = c + TOTAL_COLS;

		if( b / TOTAL_COLS > 0 &&
		    b / TOTAL_COLS < TOTAL_ROWS - 2 &&
		    mGridSquares[ a ].getState() != SQUARE_STATE_STILL &&
		    mGridSquares[ b ].getState() != SQUARE_STATE_STILL &&
		    mGridSquares[ c ].getState() != SQUARE_STATE_STILL && 
		    mGridSquares[ d ].getState() != SQUARE_STATE_STILL )
		{
		  Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
		  Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
		  Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
		  Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );
	   
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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );
	    
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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	Mix_PlayChannel( 2, gMoveSound, 0 );
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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );
	
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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
		  Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
		  Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
		  Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
		  Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	Mix_PlayChannel( 2, gMoveSound, 0 );
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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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
	    Mix_PlayChannel( 2, gMoveSound, 0 );

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
	      Mix_PlayChannel( 2, gMoveSound, 0 );

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
		Mix_PlayChannel( 2, gMoveSound, 0 );

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

int Tetromino::getType()
{
  return mType;
}

void Tetromino::clear()
{
  mType = -1;
  
  mGridSquares = NULL;

  for( int i = 0; i < 4; i++ )
  {
    mGridPositions[ i ] = -1;
    mGhostPositions[ i ] = -1;
  }

  mRotation = 0;
  mStartTime = 0;
  mCurrentTime = 0;
  mFallDelay = 0;
  mInitialFallDelay = 0;
}

void Tetromino::draw()
{
  if( mGridSquares != NULL )
  {
    mGhostPositions[ 0 ] = mGridPositions[ 0 ];
    mGhostPositions[ 1 ] = mGridPositions[ 1 ];
    mGhostPositions[ 2 ] = mGridPositions[ 2 ];
    mGhostPositions[ 3 ] = mGridPositions[ 3 ];
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

    for( int i = 0; i < 4; i++ )
    {
      mGridSquares[ mGhostPositions[ i ] ].fill( mType );
      mGridSquares[ mGhostPositions[ i ] ].setAlpha( 128 );
    }

    for( int i = 0; i < 4; i++ )
    {
      mGridSquares[ mGridPositions[ i ] ].fill( mType );
    } 
  }
}

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

void Tetromino::land()
{
  if( mGridSquares != NULL )
  {
    Mix_PlayChannel( 3, gLandSound, 0 );

    for( int i = 0; i < 4; i++ )
    {
      mGridSquares[ mGridPositions[ i ] ].stop();
    }
  }
}

enum GameStateFlag
{
  GAME_STATE_START,
  GAME_STATE_NEW,
  GAME_STATE_PLAY,
  GAME_STATE_PAUSE,
  GAME_STATE_GAMEOVER,
  GAME_STATE_SCORES,
  GAME_STATE_ERROR
};

class GameState
{
  public:
  GameState( Square* gridSquares, Square** nextSquares, Square** holdSquares, Square** startSquares, SDL_Rect* textAreas, SDL_Rect& listArea );
  void handleEvent( SDL_Event& e );
  void progress();
  void changeState();
  void render();
  int getState();

  private:
  bool createTetromino( int type );
  void updateNext();
  void updateHold();
  void randomize();
  GameStateFlag mCurrentState;
  GameStateFlag mNextState;
  Square* mGridSquares;
  Square** mNextSquares;
  Square** mHoldSquares;
  Square** mStartSquares;
  SDL_Point mTextCenters[ 3 ];
  SDL_Point mTextPositions[ 3 ];
  SDL_Point mListCenters[ TOTAL_SCORES ];
  SDL_Point mListPositions[ TOTAL_SCORES ];
  SDL_Point mNewPosition;
  Tetromino* mTetromino;
  int mNext[ 3 ];
  int mHold;
  int mScore;
  int mLines;
  int mLevel;
  int mNewRank;
  int mDelay[ 30 ];
  int mRandom[ TOTAL_SQUARES ];
  int mCurrentBG;
  int mCurrentBGM;
  bool mClearing;
  bool mTetris;
  bool mNeedToHold;
  bool mHolding;
  bool mScoreChanged;
  bool mLinesChanged;
  bool mLevelChanged;
  bool mNewScore;
  bool mNameChanged;
  bool mMuted;
  std::string mName;
  Uint32 mStartTime;
  Uint32 mCurrentTime;
  Uint32 mPauseTime;
};

GameState::GameState( Square* gridSquares, Square** nextSquares, Square** holdSquares, Square** startSquares, SDL_Rect* textAreas, SDL_Rect& listArea )
{
  mCurrentState = GAME_STATE_START;
  mNextState = GAME_STATE_START;

  mGridSquares = gridSquares;
  mNextSquares = nextSquares;
  mHoldSquares = holdSquares;
  mStartSquares = startSquares;

  for( int i = 0; i < 3; i++ )
  {
    mTextCenters[ i ].x = textAreas[ i ].x + ( textAreas[ i ].w / 2 );
    mTextCenters[ i ].y = textAreas[ i ].y + ( textAreas[ i ].h / 2 );
    mTextPositions[ i ] = mTextCenters[ i ];
  }

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

  mNewPosition = mListCenters[ TOTAL_SCORES / 2 ];

  mTetromino = NULL;
  mNext[ 0 ] = -1;
  mNext[ 1 ] = -1;
  mNext[ 2 ] = -1;

  mHold = -1;

  mScore = 0;
  mLines = 0;
  mLevel = 0;

  mNewRank = -1;

  mScoreChanged = false;
  mLinesChanged = false;
  mLevelChanged = false;

  mNewScore = false;
  mNameChanged = false;
  mName = "";
  
  mDelay[ 0 ] = 800;
  mDelay[ 1 ] = 720;
  mDelay[ 2 ] = 630;
  mDelay[ 3 ] = 550;
  mDelay[ 4 ] = 470;
  mDelay[ 5 ] = 380;
  mDelay[ 6 ] = 300;
  mDelay[ 7 ] = 220;
  mDelay[ 8 ] = 130;
  mDelay[ 9 ] = 100;
  mDelay[ 10 ] = 80;
  mDelay[ 11 ] = 80;
  mDelay[ 12 ] = 80;
  mDelay[ 13 ] = 70;
  mDelay[ 14 ] = 70;
  mDelay[ 15 ] = 70;
  mDelay[ 16 ] = 50;
  mDelay[ 17 ] = 50;
  mDelay[ 18 ] = 50;
  mDelay[ 19 ] = 30;
  mDelay[ 20 ] = 30;
  mDelay[ 21 ] = 30;
  mDelay[ 22 ] = 30;
  mDelay[ 23 ] = 30;
  mDelay[ 24 ] = 30;
  mDelay[ 25 ] = 30;
  mDelay[ 26 ] = 30;
  mDelay[ 27 ] = 30;
  mDelay[ 28 ] = 30;

  for( int i = 0; i < TOTAL_SQUARES; i++ )
  {
    mRandom[ i ] = -1;
  }

  mCurrentBG = 0;
  mCurrentBGM = 0;

  mClearing = false;
  mTetris = false;
  mNeedToHold = false;
  mHolding = false;

  mStartTime = SDL_GetTicks();
  mCurrentTime = mStartTime;
  mPauseTime = 0;

  mMuted = false;

  srand( time( NULL ) );
  rand();
}

void GameState::handleEvent( SDL_Event& e )
{
  if( e.type == SDL_KEYDOWN )
  {
    if( e.key.keysym.sym == SDLK_RETURN )
    {
      if( mCurrentState == GAME_STATE_START )
      {
	mCurrentBG = rand() % TOTAL_BGS;

	mNextState = GAME_STATE_NEW;
	mStartTime = SDL_GetTicks();
	mCurrentTime = mStartTime;
      }
      if( mCurrentState == GAME_STATE_SCORES && mNewScore && mName.length() > 0 )
      {
	mNewScore = false;

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

	mStartTime = SDL_GetTicks();
	mCurrentTime = mStartTime;
      }
    }
    else if( e.key.keysym.sym == SDLK_ESCAPE && e.key.repeat == 0 )
    {
      if( mCurrentState == GAME_STATE_PLAY )
      {
	Mix_Pause( -1 );

	mNextState = GAME_STATE_PAUSE;
	mPauseTime = SDL_GetTicks();
	mCurrentTime = mPauseTime;
      }

      if( mCurrentState == GAME_STATE_PAUSE )
      {
	Mix_Resume( -1 );

	mNextState = GAME_STATE_PLAY;
	mStartTime += mCurrentTime;
      }
    }
    else if( e.key.keysym.sym == SDLK_c )
    {
      if( mCurrentState == GAME_STATE_PLAY && !mClearing && !mHolding )
      {
	Mix_PlayChannel( 6, gHoldSound, 0 );

	mNeedToHold = true;
	mTetromino->handleEvent( e );
      }
    }
    else if( e.key.keysym.sym == SDLK_BACKSPACE )
    {
      if( mCurrentState == GAME_STATE_SCORES && mNewScore && mName.length() > 0 )
      {
	mName.pop_back();
	mNameChanged = true;
      }
    }
    else if( e.key.keysym.sym == SDLK_1 && e.key.repeat == 0 )
    {
      if( mMuted )
      {
	Mix_Volume( -1, MIX_MAX_VOLUME );
	mMuted = false;
      }
      else
      {
	Mix_Volume( -1, 0 );
	mMuted = true;
      }
    }
    else
    {
      if( mCurrentState == GAME_STATE_PLAY && !mClearing )
      {
	mTetromino->handleEvent( e );
      }
    }
  }
  else if( e.type == SDL_KEYUP )
  {
    if( mCurrentState == GAME_STATE_PLAY && !mClearing )
    {
      mTetromino->handleEvent( e );
    }
  }
  else if( e.type == SDL_TEXTINPUT )
  {
    if( mCurrentState == GAME_STATE_SCORES && mNewScore && mName.length() < 10 )
    {
      mName += e.text.text;
      mNameChanged = true;
    }
  }
}

void GameState::progress()
{
  if( mCurrentState == GAME_STATE_START )
  {
    mCurrentTime = SDL_GetTicks() - mStartTime;
    if( mCurrentTime >= 20000 )
    {
      mStartTime = SDL_GetTicks();
    }
    
    if( Mix_Playing( 0 ) == 0 )
    {
      Mix_PlayChannel( 0, gStartMusic, -1 );
    }
  }

  if( mCurrentState == GAME_STATE_NEW )
  {
    if( Mix_Playing( 0 ) == 1 )
    {
      Mix_FadeOutChannel( 0, 2000 );
    }

    mCurrentTime = SDL_GetTicks() - mStartTime;
    if( mCurrentTime >= 3000 )
    {
      mCurrentBGM = rand() % TOTAL_BGM;

      for( int i = 0; i < TOTAL_SQUARES; i++ )
      {
	mGridSquares[ i ].clear();
      }
      for( int i = 0; i < 8; i++ )
      {
	mNextSquares[ 0 ][ i ].clear();
	mNextSquares[ 2 ][ i ].clear();
	mNextSquares[ 4 ][ i ].clear();

	mHoldSquares[ 0 ][ i ].clear();
      }
      for( int i = 0; i < 6; i++ )
      {
	mNextSquares[ 1 ][ i ].clear();
	mNextSquares[ 3 ][ i ].clear();
	mNextSquares[ 5 ][ i ].clear();

	mHoldSquares[ 1 ][ i ].clear();
      }

      for( int i = 0; i < 3; i++ )
      {
	mTextPositions[ i ] = mTextCenters[ i ];
      }

      mNewPosition = mListCenters[ TOTAL_SCORES / 2 ];

      mHold = -1;

      mScore = 0;
      mLines = 0;
      mLevel = 0;

      mNewRank = -1;

      mScoreChanged = true;
      mLinesChanged = true;
      mLevelChanged = true;

      mNewScore = false;
      mNameChanged = true;

      randomize();

      int first = rand() % TETROMINO_COUNT;
      if( !createTetromino( first ) )
      {
	mNextState = GAME_STATE_ERROR;
	printf( "Error: Could not generate first tetromino!\n" );
      }
      else
      {
	mNext[ 1 ] = rand() % ( TETROMINO_COUNT + 1 );
	if( mNext[ 1 ] == first || mNext[ 1 ] == TETROMINO_COUNT )
	{
	  mNext[ 1 ] = rand() % TETROMINO_COUNT;
	}
	mNext[ 2 ] = rand() % ( TETROMINO_COUNT + 1 );
	if( mNext[ 2 ] == mNext[ 1 ] || mNext[ 2 ] == TETROMINO_COUNT )
	{
	  mNext[ 2 ] = rand() % TETROMINO_COUNT;
	}
	updateNext();
	mNextState = GAME_STATE_PLAY;
      }
    }
  }

  if( mCurrentState == GAME_STATE_PLAY )
  {
    if( Mix_Playing( 0 ) == 0 )
    {
      Mix_PlayChannel( 0, gBGMusic[ mCurrentBGM ], -1 );
    }

    if( !mClearing )
    {
      if( mNeedToHold )
      {
	mNeedToHold = false;
	mHolding = true;

	int next;

	if( mHold == -1 )
	{
	  next = mNext[ 0 ];
	  updateNext();
	}
	else
	{
	  next = mHold;
	}

	mHold = mTetromino->getType();

	delete mTetromino;

	if( !createTetromino( next ) )
	{
	  mNextState = GAME_STATE_GAMEOVER;

	  mStartTime = SDL_GetTicks();
	  mCurrentTime = mStartTime;
	}
	else
	{
	  updateHold();
	}
      }	
      else if( !( mTetromino->fall() ) )
      {
	delete mTetromino;

	mScore += 10 * ( mLevel + 1 );
	mScoreChanged = true;

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

	  mLines += lines;
	  mLinesChanged = true;

	  if( lines == 1 )
	  {
	    mScore += 50 * ( mLevel + 1 );
	  }
	  else if( lines == 2 )
	  {
	    mScore += 150 * ( mLevel + 1 );
	  }
	  else if( lines == 3 )
	  {
	    mScore += 350 * ( mLevel + 1 );
	  }
	  else if( lines == 4 )
	  {
	    mScore += 1000 * ( mLevel + 1 );
	    mTetris = true;
	  }

	  if( mTetris )
	  {
	    Mix_PlayChannel( 4, gTetrisSound, 0 );
	  }
	  else
	  {
	    Mix_PlayChannel( 5, gClearSound, 0 );
	  }

	  mScoreChanged = true;
	
	  mStartTime = SDL_GetTicks();
	  mCurrentTime = mStartTime;
	}
	else
	{ 
	  if( !createTetromino( mNext[ 0 ] ) )
	  {
	    mNextState = GAME_STATE_GAMEOVER;

	    mStartTime = SDL_GetTicks();
	    mCurrentTime = mStartTime;
	  }
	  else
	  {
	    updateNext();
	  }
	}
      }
    }
    else
    {
      mCurrentTime = SDL_GetTicks() - mStartTime;
      if( mCurrentTime >= 500 )
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

	if( !createTetromino( mNext[ 0 ] ) )
	{
	  mNextState = GAME_STATE_GAMEOVER;

	  mStartTime = SDL_GetTicks();
	  mCurrentTime = 0;
	}
	else
	{
	  mNextState = GAME_STATE_PLAY;
	  updateNext();
	}
      }
      else
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
	      mGridSquares[ j ].setAlpha( 255 - ( 255 * mCurrentTime / 500 ) );
	    }
	  }
	}
      }
    }
  }
    
  if( mCurrentState == GAME_STATE_PAUSE )
  {
    mCurrentTime = SDL_GetTicks() - mPauseTime;
  }

  if( mCurrentState == GAME_STATE_GAMEOVER )
  {
    Mix_PlayChannel( -1, gGameOverSound, 0 );

    if( Mix_Playing( -1 ) > 0 )
    {
      Mix_FadeOutChannel( -1, 7000 );
    }

    mCurrentTime = SDL_GetTicks() - mStartTime;
    if( mCurrentTime >= 7000 )
    {
      mNextState = GAME_STATE_SCORES;
      
      for( int i = TOTAL_SCORES - 1; i >= 0 && mScore > gScores[ i ].score; i-- )
      {
	mNewScore = true;
	mNewRank = i;
      }

      if( mNewScore )
      {
	for( int i = TOTAL_SCORES - 1; i > mNewRank; i-- )
	{
	  strcpy( gScores[ i ].name, gScores[ i - 1 ].name );
	  gScores[ i ].score = gScores[ i - 1 ].score;
	}
      
	gScores[ mNewRank ].score = mScore;

	SDL_Color scoreColor = { 0, 0, 0 };

	std::string score = std::to_string( mScore );
	gNewTextTextures[ 1 ].loadFromRenderedText( score.c_str(), scoreColor ); 
      }

      for( int i = 0; i < TOTAL_SCORES; i++ )
      {
	gListTextTextures[ i ].setAlpha( 0 );
	gListTextTextures[ i + TOTAL_SCORES ].setAlpha( 0 );
      }

      mStartTime = SDL_GetTicks();
      mCurrentTime = 0;
    }
  }

  if( mCurrentState == GAME_STATE_SCORES )
  {
    if( Mix_Playing( 0 ) == 0 )
    {
      Mix_PlayChannel( 0, gScoreMusic, -1 );
    }

    mCurrentTime = SDL_GetTicks() - mStartTime;
    if( mNewScore )
    {
      strcpy( gScores[ mNewRank ].name, mName.c_str() ); 
    }

    if( !mNewScore && mCurrentTime >= 8000 )
    {
      if( Mix_Playing( 0 ) == 1 )
      {
	Mix_FadeOutChannel( 0, 2000 );
      }
    }

    if( !mNewScore && mCurrentTime >= 10000 )
    {
      mNextState = GAME_STATE_START;

      mStartTime = SDL_GetTicks();
      mCurrentTime = 0;
    }
  }

  if( mCurrentState == GAME_STATE_ERROR )
  {
  }
}

void GameState::changeState()
{
  mCurrentState = mNextState;
}

void GameState::render()
{
  SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
  SDL_RenderClear( gRenderer );
  
  if( mCurrentState == GAME_STATE_START )
  {
    int alpha = 255;
    int frame = 4;
    if( mCurrentTime < 250 )
    {
      alpha = 255 * mCurrentTime / 250;
      frame = mCurrentTime / 50;
    }
    else if( mCurrentTime >= 9750 )
    {
      alpha = 255 - ( 255 * ( mCurrentTime - 9750 ) / 250 );
      frame = 4 - ( ( mCurrentTime - 9750 ) / 50 );
    }

    if( mCurrentTime < 10000 )
    {
      gHandBlackTexture.setAlpha( alpha );
      gHandBlackTexture.render( 0, ( SCREEN_HEIGHT / 2 ) - 138, &gHandClips[ frame ] );
    }

    gBlankBGTexture.render( 0, 0 );
   
    for( int i = 0; i < 8; i++ )
    {
      mStartSquares[ 0 ][ i ].clear();
      mStartSquares[ 2 ][ i ].clear();
    }

    for( int i = 0; i < 6; i++ )
    {
      mStartSquares[ 1 ][ i ].clear();
      mStartSquares[ 3 ][ i ].clear();
    }
   
    int type = ( mCurrentTime / 100 ) % TETROMINO_COUNT;
    alpha = 255;
    if( mCurrentTime < 2000 )
    {
      alpha = 255 * mCurrentTime / 2000;
    }
    else if( mCurrentTime > 8000 )
    {
      alpha = 255 - ( 255 * ( mCurrentTime - 8000 ) / 2000 );
    } 

    if( mCurrentTime < 10000 )
    {
      switch( type )
      {
        case TETROMINO_I:
	  mStartSquares[ 0 ][ 4 ].fill( TETROMINO_I );
	  mStartSquares[ 0 ][ 5 ].fill( TETROMINO_I );
	  mStartSquares[ 0 ][ 6 ].fill( TETROMINO_I );
	  mStartSquares[ 0 ][ 7 ].fill( TETROMINO_I );

	  mStartSquares[ 2 ][ 4 ].fill( TETROMINO_I );
	  mStartSquares[ 2 ][ 5 ].fill( TETROMINO_I );
	  mStartSquares[ 2 ][ 6 ].fill( TETROMINO_I );
	  mStartSquares[ 2 ][ 7 ].fill( TETROMINO_I );

	  mStartSquares[ 0 ][ 4 ].setAlpha( alpha );
	  mStartSquares[ 0 ][ 5 ].setAlpha( alpha );
	  mStartSquares[ 0 ][ 6 ].setAlpha( alpha );
	  mStartSquares[ 0 ][ 7 ].setAlpha( alpha );

	  mStartSquares[ 2 ][ 4 ].setAlpha( alpha );
	  mStartSquares[ 2 ][ 5 ].setAlpha( alpha );
	  mStartSquares[ 2 ][ 6 ].setAlpha( alpha );
	  mStartSquares[ 2 ][ 7 ].setAlpha( alpha );

	  mStartSquares[ 0 ][ 4 ].render();
	  mStartSquares[ 0 ][ 5 ].render();
	  mStartSquares[ 0 ][ 6 ].render();
	  mStartSquares[ 0 ][ 7 ].render();

	  mStartSquares[ 2 ][ 4 ].render();
	  mStartSquares[ 2 ][ 5 ].render();
	  mStartSquares[ 2 ][ 6 ].render();
	  mStartSquares[ 2 ][ 7 ].render();
	  break;

        case TETROMINO_J:
	  mStartSquares[ 1 ][ 0 ].fill( TETROMINO_J );
	  mStartSquares[ 1 ][ 3 ].fill( TETROMINO_J );
	  mStartSquares[ 1 ][ 4 ].fill( TETROMINO_J );
	  mStartSquares[ 1 ][ 5 ].fill( TETROMINO_J );

	  mStartSquares[ 3 ][ 0 ].fill( TETROMINO_J );
	  mStartSquares[ 3 ][ 3 ].fill( TETROMINO_J );
	  mStartSquares[ 3 ][ 4 ].fill( TETROMINO_J );
	  mStartSquares[ 3 ][ 5 ].fill( TETROMINO_J );

	  mStartSquares[ 1 ][ 0 ].setAlpha( alpha );
	  mStartSquares[ 1 ][ 3 ].setAlpha( alpha );
	  mStartSquares[ 1 ][ 4 ].setAlpha( alpha );
	  mStartSquares[ 1 ][ 5 ].setAlpha( alpha );

	  mStartSquares[ 3 ][ 0 ].setAlpha( alpha );
	  mStartSquares[ 3 ][ 3 ].setAlpha( alpha );
	  mStartSquares[ 3 ][ 4 ].setAlpha( alpha );
	  mStartSquares[ 3 ][ 5 ].setAlpha( alpha );

	  mStartSquares[ 1 ][ 0 ].render();
	  mStartSquares[ 1 ][ 3 ].render();
	  mStartSquares[ 1 ][ 4 ].render();
	  mStartSquares[ 1 ][ 5 ].render();

	  mStartSquares[ 3 ][ 0 ].render();
	  mStartSquares[ 3 ][ 3 ].render();
	  mStartSquares[ 3 ][ 4 ].render();
	  mStartSquares[ 3 ][ 5 ].render();
	  break;

        case TETROMINO_L:
	  mStartSquares[ 1 ][ 2 ].fill( TETROMINO_L );
	  mStartSquares[ 1 ][ 3 ].fill( TETROMINO_L );
	  mStartSquares[ 1 ][ 4 ].fill( TETROMINO_L );
	  mStartSquares[ 1 ][ 5 ].fill( TETROMINO_L );

	  mStartSquares[ 3 ][ 2 ].fill( TETROMINO_L );
	  mStartSquares[ 3 ][ 3 ].fill( TETROMINO_L );
	  mStartSquares[ 3 ][ 4 ].fill( TETROMINO_L );
	  mStartSquares[ 3 ][ 5 ].fill( TETROMINO_L );

	  mStartSquares[ 1 ][ 2 ].setAlpha( alpha );
	  mStartSquares[ 1 ][ 3 ].setAlpha( alpha );
	  mStartSquares[ 1 ][ 4 ].setAlpha( alpha );
	  mStartSquares[ 1 ][ 5 ].setAlpha( alpha );

	  mStartSquares[ 3 ][ 2 ].setAlpha( alpha );
	  mStartSquares[ 3 ][ 3 ].setAlpha( alpha );
	  mStartSquares[ 3 ][ 4 ].setAlpha( alpha );
	  mStartSquares[ 3 ][ 5 ].setAlpha( alpha );

	  mStartSquares[ 1 ][ 2 ].render();
	  mStartSquares[ 1 ][ 3 ].render();
	  mStartSquares[ 1 ][ 4 ].render();
	  mStartSquares[ 1 ][ 5 ].render();

	  mStartSquares[ 3 ][ 2 ].render();
	  mStartSquares[ 3 ][ 3 ].render();
	  mStartSquares[ 3 ][ 4 ].render();
	  mStartSquares[ 3 ][ 5 ].render();
	  break;

        case TETROMINO_O:
	  mStartSquares[ 0 ][ 1 ].fill( TETROMINO_O );
	  mStartSquares[ 0 ][ 2 ].fill( TETROMINO_O );
	  mStartSquares[ 0 ][ 5 ].fill( TETROMINO_O );
	  mStartSquares[ 0 ][ 6 ].fill( TETROMINO_O );

	  mStartSquares[ 2 ][ 1 ].fill( TETROMINO_O );
	  mStartSquares[ 2 ][ 2 ].fill( TETROMINO_O );
	  mStartSquares[ 2 ][ 5 ].fill( TETROMINO_O );
	  mStartSquares[ 2 ][ 6 ].fill( TETROMINO_O );

	  mStartSquares[ 0 ][ 1 ].setAlpha( alpha );
	  mStartSquares[ 0 ][ 2 ].setAlpha( alpha );
	  mStartSquares[ 0 ][ 5 ].setAlpha( alpha );
	  mStartSquares[ 0 ][ 6 ].setAlpha( alpha );

	  mStartSquares[ 2 ][ 1 ].setAlpha( alpha );
	  mStartSquares[ 2 ][ 2 ].setAlpha( alpha );
	  mStartSquares[ 2 ][ 5 ].setAlpha( alpha );
	  mStartSquares[ 2 ][ 6 ].setAlpha( alpha );

	  mStartSquares[ 0 ][ 1 ].render();
	  mStartSquares[ 0 ][ 2 ].render();
	  mStartSquares[ 0 ][ 5 ].render();
	  mStartSquares[ 0 ][ 6 ].render();

	  mStartSquares[ 2 ][ 1 ].render();
	  mStartSquares[ 2 ][ 2 ].render();
	  mStartSquares[ 2 ][ 5 ].render();
	  mStartSquares[ 2 ][ 6 ].render();
	  break;

        case TETROMINO_S:
	  mStartSquares[ 1 ][ 1 ].fill( TETROMINO_S );
	  mStartSquares[ 1 ][ 2 ].fill( TETROMINO_S );
	  mStartSquares[ 1 ][ 3 ].fill( TETROMINO_S );
	  mStartSquares[ 1 ][ 4 ].fill( TETROMINO_S );

	  mStartSquares[ 3 ][ 1 ].fill( TETROMINO_S);
	  mStartSquares[ 3 ][ 2 ].fill( TETROMINO_S );
	  mStartSquares[ 3 ][ 3 ].fill( TETROMINO_S );
	  mStartSquares[ 3 ][ 4 ].fill( TETROMINO_S );

	  mStartSquares[ 1 ][ 1 ].setAlpha( alpha );
	  mStartSquares[ 1 ][ 2 ].setAlpha( alpha );
	  mStartSquares[ 1 ][ 3 ].setAlpha( alpha );
	  mStartSquares[ 1 ][ 4 ].setAlpha( alpha );

	  mStartSquares[ 3 ][ 1 ].setAlpha( alpha );
	  mStartSquares[ 3 ][ 2 ].setAlpha( alpha );
	  mStartSquares[ 3 ][ 3 ].setAlpha( alpha );
	  mStartSquares[ 3 ][ 4 ].setAlpha( alpha );

	  mStartSquares[ 1 ][ 1 ].render();
	  mStartSquares[ 1 ][ 2 ].render();
	  mStartSquares[ 1 ][ 3 ].render();
	  mStartSquares[ 1 ][ 4 ].render();

	  mStartSquares[ 3 ][ 1 ].render();
	  mStartSquares[ 3 ][ 2 ].render();
	  mStartSquares[ 3 ][ 3 ].render();
	  mStartSquares[ 3 ][ 4 ].render();
	  break;

        case TETROMINO_T:
	  mStartSquares[ 1 ][ 1 ].fill( TETROMINO_T );
	  mStartSquares[ 1 ][ 3 ].fill( TETROMINO_T );
	  mStartSquares[ 1 ][ 4 ].fill( TETROMINO_T );
	  mStartSquares[ 1 ][ 5 ].fill( TETROMINO_T );

	  mStartSquares[ 3 ][ 1 ].fill( TETROMINO_T );
	  mStartSquares[ 3 ][ 3 ].fill( TETROMINO_T );
	  mStartSquares[ 3 ][ 4 ].fill( TETROMINO_T );
	  mStartSquares[ 3 ][ 5 ].fill( TETROMINO_T );

	  mStartSquares[ 1 ][ 1 ].setAlpha( alpha );
	  mStartSquares[ 1 ][ 3 ].setAlpha( alpha );
	  mStartSquares[ 1 ][ 4 ].setAlpha( alpha );
	  mStartSquares[ 1 ][ 5 ].setAlpha( alpha );

	  mStartSquares[ 3 ][ 1 ].setAlpha( alpha );
	  mStartSquares[ 3 ][ 3 ].setAlpha( alpha );
	  mStartSquares[ 3 ][ 4 ].setAlpha( alpha );
	  mStartSquares[ 3 ][ 5 ].setAlpha( alpha );

	  mStartSquares[ 1 ][ 1 ].render();
	  mStartSquares[ 1 ][ 3 ].render();
	  mStartSquares[ 1 ][ 4 ].render();
	  mStartSquares[ 1 ][ 5 ].render();

	  mStartSquares[ 3 ][ 1 ].render();
	  mStartSquares[ 3 ][ 3 ].render();
	  mStartSquares[ 3 ][ 4 ].render();
	  mStartSquares[ 3 ][ 5 ].render();
	  break; 

        case TETROMINO_Z:
	  mStartSquares[ 1 ][ 0 ].fill( TETROMINO_Z );
	  mStartSquares[ 1 ][ 1 ].fill( TETROMINO_Z );
	  mStartSquares[ 1 ][ 4 ].fill( TETROMINO_Z );
	  mStartSquares[ 1 ][ 5 ].fill( TETROMINO_Z );
	  
	  mStartSquares[ 3 ][ 0 ].fill( TETROMINO_Z );
	  mStartSquares[ 3 ][ 1 ].fill( TETROMINO_Z );
	  mStartSquares[ 3 ][ 4 ].fill( TETROMINO_Z );
	  mStartSquares[ 3 ][ 5 ].fill( TETROMINO_Z );

	  mStartSquares[ 1 ][ 0 ].setAlpha( alpha );
	  mStartSquares[ 1 ][ 1 ].setAlpha( alpha );
	  mStartSquares[ 1 ][ 4 ].setAlpha( alpha );
	  mStartSquares[ 1 ][ 5 ].setAlpha( alpha );
	  
	  mStartSquares[ 3 ][ 0 ].setAlpha( alpha );
	  mStartSquares[ 3 ][ 1 ].setAlpha( alpha );
	  mStartSquares[ 3 ][ 4 ].setAlpha( alpha );
	  mStartSquares[ 3 ][ 5 ].setAlpha( alpha );

	  mStartSquares[ 1 ][ 0 ].render();
	  mStartSquares[ 1 ][ 1 ].render();
	  mStartSquares[ 1 ][ 4 ].render();
	  mStartSquares[ 1 ][ 5 ].render();

	  mStartSquares[ 3 ][ 0 ].render();
	  mStartSquares[ 3 ][ 1 ].render();
	  mStartSquares[ 3 ][ 4 ].render();
	  mStartSquares[ 3 ][ 5 ].render();
	  break;

        default:
	  mNextState = GAME_STATE_ERROR;
      }
    }

    if( mCurrentTime < 10000 )
    {
      gPressEnterTexture.setAlpha( 255 - ( 255 * ( ( mCurrentTime % 2000 ) - 1000 ) * ( ( mCurrentTime % 2000 ) - 1000 ) / 1000000 ) );
      gPressEnterTexture.render( 0, 0 );
    }
    else
    {
      int x = 275, y = 25;
      
      if( mCurrentTime < 20000 )
      {
	for( int i = 0; i < TOTAL_SCORES; i++ )
	{
	  int limit = 1000 * i ;
	  if( ( mCurrentTime - 10000 ) < limit )
	  {
	    gListTexture.setAlpha( 0 );
	  }
	  else if( ( mCurrentTime - 10000 ) >= limit && ( mCurrentTime - 10000 ) < ( limit + 1000 ) )
	  {
	    int a = 255 * ( ( mCurrentTime - 10000 ) - limit ) / 1000;
	    gListTexture.setAlpha( a );
	    gListTextTextures[ i ].setAlpha( a );
	    gListTextTextures[ i + TOTAL_SCORES ].setAlpha( a );
	  }
	  else if( ( mCurrentTime - 10000 ) >= ( limit + 1000 ) && ( mCurrentTime - 10000 ) < 9000 )
	  {
	    gListTexture.setAlpha( 255 );
	  }
	  else
	  {
	    gListTexture.setAlpha( 255 - ( 255 * ( ( mCurrentTime - 10000 ) - 9000 ) / 1000 ) );
	    gListTextTextures[ i ].setAlpha( 255 - ( 255 * ( ( mCurrentTime - 10000 ) - 9000 ) / 1000 ) );
	    gListTextTextures[ i + TOTAL_SCORES ].setAlpha( 255 - ( 255 * ( ( mCurrentTime - 10000 ) - 9000 ) / 1000 ) );
	  }
	  gListTexture.render( x, y + ( i * 100 ), &gListClips[ i ] );
	  gListTextTextures[ i ].render( mListPositions[ i ].x, mListPositions[ i ].y );
	  gListTextTextures[ i + TOTAL_SCORES ].render( mListCenters[ i ].x + 13, mListCenters[ i ].y + 13 );
	}
      }
    }
  }

  if( mCurrentState == GAME_STATE_NEW )
  {
    gBlankBGTexture.setAlpha( 255 );
    gPressEnterTexture.setAlpha( 255 );
    
    gBGTextures[ mCurrentBG ].setAlpha( 255 * mCurrentTime / 3000 );
    gBGTextures[ mCurrentBG ].render( 0, 0 );
    gPlayBGTexture.setAlpha( 255 * mCurrentTime / 3000 );
    gPlayBGTexture.render( 0, 0 );
    gPlayBGTexture.setAlpha( 255 );

    gPausedTexture.setAlpha( 255 );
    gGameOverTexture.setAlpha( 0 );
    gEnterNameTexture.setAlpha( 255 );
    gListTexture.setAlpha( 255 );
    gSquareSpriteTexture.setAlpha( 255 );
  }

  if( mCurrentState == GAME_STATE_PLAY )
  {
    if( mLevelChanged )
    {
      if( mLevel != 0 )
      {
	Mix_FadeOutChannel( 0, 500 );

	int nextBGM = rand() % TOTAL_BGM;
	while( nextBGM == mCurrentBGM )
	{
	  nextBGM = rand() % TOTAL_BGM;
	}
	mCurrentBGM = nextBGM;

	int nextBG = rand() % TOTAL_BGS;
	while( nextBG == mCurrentBG )
	{
	  nextBG = rand() % TOTAL_BGS;
	}
	mCurrentBG = nextBG;
      }
    }
    gBGTextures[ mCurrentBG ].setAlpha( 255 );
    gBGTextures[ mCurrentBG ].render( 0, 0 );

    if( mClearing )
    {
      int alpha = 255 * mCurrentTime / 500;
      gBlackTexture.setAlpha( alpha );
      gBlackTexture.render( 0, 0 );

      int frame = 4;

      if( mCurrentTime < 250 )
      {
	frame = mCurrentTime / 50;
      }
      
      gHandWhiteTexture.render( 0, 0, &gHandClips[ frame ] );
      gHandWhiteTexture.render( 0, 275, &gHandClips[ frame ] );
    }
 
    gPlayBGTexture.render( 0, 0 );

    if( mTetris )
    {
      int alpha = 255 * ( mCurrentTime % 100 ) / 100;
      gTetrisTexture.setAlpha( alpha );
      gTetrisTexture.render( 0, 0 );
    }
    
    SDL_Color textColor = { 0, 0, 0 };
    
    if( mScoreChanged )
    {
      std::string score = std::to_string( mScore );
      gScoreTextTexture.loadFromRenderedText( score.c_str(), textColor );
      mTextPositions[ 0 ].x = mTextCenters[ 0 ].x - ( gScoreTextTexture.getWidth() / 2 );
      mTextPositions[ 0 ].y = mTextCenters[ 0 ].y - ( gScoreTextTexture.getHeight() / 2 );
      mScoreChanged = false;
    }
    
    gScoreTextTexture.render( mTextPositions[ 0 ].x, mTextPositions[ 0 ].y );
      
    if( mLinesChanged )
    {
      std::string lines = std::to_string( mLines );
      gLinesTextTexture.loadFromRenderedText( lines.c_str(), textColor );
      mTextPositions[ 1 ].x = mTextCenters[ 1 ].x - ( gLinesTextTexture.getWidth() / 2 );
      mTextPositions[ 1 ].y = mTextCenters[ 1 ].y - ( gLinesTextTexture.getHeight() / 2 );
      mLinesChanged = false;
    }

    gLinesTextTexture.render( mTextPositions[ 1 ].x, mTextPositions[ 1 ].y );

    if( mLevelChanged )
    {
      std::string level = std::to_string( mLevel );
      gLevelTextTexture.loadFromRenderedText( level.c_str(), textColor );
      mTextPositions[ 2 ].x = mTextCenters[ 2 ].x - ( gLevelTextTexture.getWidth() / 2 );
      mTextPositions[ 2 ].y = mTextCenters[ 2 ].y - ( gLevelTextTexture.getHeight() / 2 );
      mLevelChanged = false;
    }

    gLevelTextTexture.render( mTextPositions[ 2 ].x, mTextPositions[ 2 ].y );
      
    for( int i = ( 2 * TOTAL_COLS ); i < TOTAL_SQUARES; i++ )
    {
      if( mGridSquares[ i ].getState() != SQUARE_STATE_BLANK )
      {
	mGridSquares[ i ].render();
      }
    }

    switch( mNext[ 0 ] )
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

    switch( mNext[ 1 ] )
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

    switch( mNext[ 2 ] )
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

    switch( mHold )
    {
      case -1:
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
  }

  if( mCurrentState == GAME_STATE_PAUSE )
  {
    gBGTextures[ mCurrentBG ].render( 0, 0 );

    if( mClearing )
    {
      gBlackTexture.render( 0, 0 );

      int frame = 4;
      if( mPauseTime < 250 )
      {
	frame = mPauseTime / 50;
      }
      gHandWhiteTexture.render( 0, 0, &gHandClips[ frame ] );
      gHandWhiteTexture.render( 0, 275, &gHandClips[ frame ] );
    }

    gPlayBGTexture.render( 0, 0 );

    if( mTetris )
    {
      int alpha = 255 * ( mPauseTime % 100 ) / 100;
      gTetrisTexture.setAlpha( alpha );
      gTetrisTexture.render( 0, 0 );
    }

    gPausedTexture.setAlpha( 255 - ( 255 * ( ( mCurrentTime % 2000 ) - 1000 ) * ( ( mCurrentTime % 2000 ) - 1000 ) / 1000000 ) );
    gPausedTexture.render( 0, 0 );
  }

  if( mCurrentState == GAME_STATE_GAMEOVER )
  {
    if( mCurrentTime < 4000 )
    {
      gBGTextures[ mCurrentBG ].setAlpha( 255 - ( 255 * mCurrentTime / 4000 ) );
      gBGTextures[ mCurrentBG ].render( 0, 0 );
      gPlayBGTexture.setAlpha( 255 - ( 255 * mCurrentTime / 4000 ) );
      gPlayBGTexture.render( 0, 0 );

      mGridSquares[ mRandom[ mCurrentTime / 20 ] ].clear();
      
      for( int i = ( 2 * TOTAL_COLS ); i < TOTAL_SQUARES; i++ )
      {
	if( mGridSquares[ i ].getState() != SQUARE_STATE_BLANK )
	{
	  mGridSquares[ i ].render();
	}
      }
    }

    gBlankBGTexture.render( 0, 0 );

    if( mCurrentTime < 3000 )
    {
      gGameOverTexture.setAlpha( 0 );
    }
    else if( mCurrentTime >= 3000 && mCurrentTime < 4000 )
    {
      gGameOverTexture.setAlpha( 255 * ( mCurrentTime - 3000 ) / 1000 );
    }
    else if( mCurrentTime >= 6000 )
    {
      gGameOverTexture.setAlpha( 255 - ( 255 * ( mCurrentTime - 6000 ) / 1000 ) );
    }
    else
    {
      gGameOverTexture.setAlpha( 255 );
    }
    gGameOverTexture.render( 0, 0 );
  }

  if( mCurrentState == GAME_STATE_SCORES )
  {
    SDL_Color nameColor = { 255, 255, 255 };
    SDL_Color scoreColor = { 0, 0, 0 };

    gBlankBGTexture.render( 0, 0 );

    if( mNewScore )
    {
      gEnterNameTexture.setAlpha( 255 - ( 255 * ( ( mCurrentTime % 2000 ) - 1000 ) * ( ( mCurrentTime % 2000 ) - 1000 ) / 1000000 ) );
      gEnterNameTexture.render( 0, 0 );

      gListTexture.render( 275, 225, &gListClips[ mNewRank ] );

      if( mNameChanged )
      {
	if( mName != "" )
	{
	  gNewTextTextures[ 0 ].loadFromRenderedText( mName.c_str(), nameColor );
	}
	else
	{
	  gNewTextTextures[ 0 ].loadFromRenderedText( " ", nameColor );
	}

	mNewPosition.x = mListCenters[ TOTAL_SCORES / 2 ].x - ( gNewTextTextures[ 0 ].getWidth() / 2 );
	mNewPosition.y = mListCenters[ TOTAL_SCORES / 2 ].y - ( gNewTextTextures[ 0 ].getHeight() / 2 );

	mNameChanged = false;
      }

      gNewTextTextures[ 0 ].render( mNewPosition.x, mNewPosition.y );
      gNewTextTextures[ 1 ].render( mListCenters[ TOTAL_SCORES / 2 ].x + 13, mListCenters[ TOTAL_SCORES / 2 ].y + 13 );
    }
    else
    {
      int x = 275, y = 25;

      for( int i = 0; i < TOTAL_SCORES; i++ )
      {
	int limit = 1000 * i;
	if( mCurrentTime < limit )
	{
	  gListTexture.setAlpha( 0 );
	}
	else if( mCurrentTime >= limit && mCurrentTime < ( limit + 1000 ) )
	{
	  int a = 255 * ( mCurrentTime - limit ) / 1000;
	  gListTexture.setAlpha( a );
	  gListTextTextures[ i ].setAlpha( a );
	  gListTextTextures[ i + TOTAL_SCORES ].setAlpha( a );
	}
	else if( mCurrentTime >= ( limit + 1000 ) && mCurrentTime < 9000 )
	{
	  gListTexture.setAlpha( 255 );
	}
	else
	{
	  gListTexture.setAlpha( 255 - ( 255 * ( mCurrentTime - 9000 ) / 1000 ) );
	  gListTextTextures[ i ].setAlpha( 255 - ( 255 * ( mCurrentTime - 9000 ) / 1000 ) );
	  gListTextTextures[ i + TOTAL_SCORES ].setAlpha( 255 - ( 255 * ( mCurrentTime - 9000 ) / 1000 ) );
	}
	gListTexture.render( x, y + ( i * 100 ), &gListClips[ i ] );
	gListTextTextures[ i ].render( mListPositions[ i ].x, mListPositions[ i ].y );
	gListTextTextures[ i + TOTAL_SCORES ].render( mListCenters[ i ].x + 13, mListCenters[ i ].y + 13 );
      }
    }
  }

  SDL_RenderPresent( gRenderer );
}

int GameState::getState()
{
  return mCurrentState;
}

bool GameState::createTetromino( int type )
{
  bool success = true;

  int delay;

  int level = mLines / 10;
  if( level != mLevel )
  {
    mLevel = level;
    mLevelChanged = true;
  }

  if( mLevel < 29 )
  {
    delay = mDelay[ mLevel ];
  }
  else
  {
    delay = 20;
  }

  mTetromino = new Tetromino( type, mGridSquares, delay );
  if( mTetromino->getType() == -1 )
  {
    delete mTetromino;
    success = false;
  }

  return success;
}

void GameState::updateNext()
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

  mNext[ 0 ] = mNext[ 1 ];
  mNext[ 1 ] = mNext[ 2 ];
  mNext[ 2 ] = rand() % ( TETROMINO_COUNT + 1 );
  if( mNext[ 2 ] == mNext[ 1 ] || mNext[ 2 ] == TETROMINO_COUNT )
  {
    mNext[ 2 ] = rand() % TETROMINO_COUNT;
  }

  switch( mNext[ 0 ] )
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

  switch( mNext[ 1 ] )
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

  switch( mNext[ 2 ] )
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

void GameState::updateHold()
{
  for( int i = 0; i < 8; i++ )
  {
    mHoldSquares[ 0 ][ i ].clear();
  }
  for( int i = 0; i < 6; i++ )
  {
    mHoldSquares[ 1 ][ i ].clear();
  }

  switch( mHold )
  {
    case -1:
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

void GameState::randomize()
{
  for( int i = 0; i < TOTAL_SQUARES; i++ )
  {
    mRandom[ i ] = i;
  }
  for( int i = 0; i < TOTAL_SQUARES - 1; i++ )
  {
    int j = rand() % TOTAL_SQUARES;
    int temp = mRandom[ i ];
    mRandom[ i ] = mRandom[ j ];
    mRandom[ j ] = temp;
  }
}

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

  for( int i = 0; i < TOTAL_BGS; i++ )
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

  for( int i = 0; i < TOTAL_BGS; i++ )
  {
    gBGTextures[ i ].free();
  }

  for( int i = 0; i < ( 2 * TOTAL_SCORES ); i++ )
  {
    gListTextTextures[ i ].free();
  }

  gNewTextTextures[ 0 ].free();
  gNewTextTextures[ 1 ].free();

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

      GameState g( gridSquares, nextSquares, holdSquares, startSquares,  textAreas, listArea );
  
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

	  g.handleEvent( e );
	}

	g.progress();

	g.changeState();
	
	if( g.getState() == GAME_STATE_ERROR )
	{
	  quit = true;
	}
	else
	{
	  g.render();
	}

	/*
	totalTime = SDL_GetTicks() - startTime;
	if( totalTime < SCREEN_TICKS_PER_FRAME )
	{
	  SDL_Delay( SCREEN_TICKS_PER_FRAME - totalTime );
	}
	*/
      }
    }
  }

  close();
  return 0;
}
