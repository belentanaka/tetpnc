#include <SDL2/SDL.h>

#include "Timer.h"

Timer::Timer()
{
  mStartTicks = 0;
  mPauseTicks = 0;

  mStarted = false;
  mPaused = false;
}

void Timer::start()
{
  mStarted = true;
  mPaused = false;
  mStartTicks = SDL_GetTicks();
  mPauseTicks = 0;
}

void Timer::stop()
{
  mStarted = false;
  mPaused = false;
  mStartTicks = 0;
  mPauseTicks = 0;
}

void Timer::pause()
{
  if( mStarted && !mPaused )
  {
    mPaused = true;

    mPauseTicks = SDL_GetTicks() - mStartTicks;
    mStartTicks = 0;
  }
}

void Timer::unpause()
{
  if( mStarted && mPaused )
  {
    mPaused = false;

    mStartTicks = SDL_GetTicks() - mPauseTicks;
    mPauseTicks = 0;
  }
}

Uint32 Timer::getTicks()
{
  Uint32 time = 0;

  if( mStarted )
  {
    if( mPaused )
    {
      time = mPauseTicks;
    }
    else
    {
      time = SDL_GetTicks() - mStartTicks;
    }
  }

  return time;
}

Uint32 Timer::getPauseTicks()
{
  Uint32 time = 0;

  if( mStarted && mPaused )
  {
    time = SDL_GetTicks() - mPauseTicks;
  }
   
  return time;
}

bool Timer::isStarted()
{
  return mStarted;
}

bool Timer::isPaused()
{
  return mPaused;
}
