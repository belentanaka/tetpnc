#ifndef LTEXTURE_H
#define LTEXTURE_H

#include <SDL2/SDL.h>
#include <stdio.h>
#include <string>

// Texture wrapper class taken from Lazy Foo SDL tutorials: http://lazyfoo.net/tutorials/SDL/
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

#endif
