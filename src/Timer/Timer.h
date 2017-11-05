#ifndef TIMER_H
#define TIMER_H

// Taken from the Lazy Foo SDL tutorials: http://lazyfoo.net/tutorials/SDL/

class Timer
{
  public:
  Timer();
  
  void start();
  void stop();
  void pause();
  void unpause();

  Uint32 getTicks();
  Uint32 getPauseTicks();

  bool isStarted();
  bool isPaused();

  private:
  Uint32 mStartTicks;
  Uint32 mPauseTicks;
  bool mStarted;
  bool mPaused;
};

#endif
