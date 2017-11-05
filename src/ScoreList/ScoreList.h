#ifndef SCORELIST_H
#define SCORELIST_H

#include <SDL2/SDL.h>
#include <string>

#include "../constants.h"
#include "../GameState/GameState.h"

class ScoreList : public GameState
{
  public:
  ScoreList( int score, SDL_Rect& listArea );
  ~ScoreList();
  
  void handleEvent( SDL_Event& e );
  void logic();
  void render();

  private:
  SDL_Point mListCenters[ TOTAL_SCORES ];
  SDL_Point mListPositions[ TOTAL_SCORES ];
  SDL_Point mNewScorePosition;
  int mNewScore;
  int mNewRank;
  std::string mName;
  bool mGotHighScore;
  bool mNameChanged;
};

#endif
