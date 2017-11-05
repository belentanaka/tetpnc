#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <SDL2/SDL.h>

#include "../constants.h"
#include "../Timer/Timer.h"

class GameState
{
  public:
  virtual void handleEvent( SDL_Event& e ) = 0;
  virtual void logic() = 0;
  virtual void render() = 0;
  GameStateFlag getNextState();

  protected:
  Timer mTimer;
  GameStateFlag mNextState;
};

#endif 
