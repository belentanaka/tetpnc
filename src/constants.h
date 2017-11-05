#ifndef CONSTANTS_H
#define CONSTANTS_H

// Screen dimensions
const int SCREEN_WIDTH = 850;
const int SCREEN_HEIGHT = 550;

// Tetris grid dimensions
const int TOTAL_ROWS = 20;
const int TOTAL_COLS = 10;
const int TOTAL_SQUARES = TOTAL_ROWS * TOTAL_COLS;

// Score, music, and background image count
const int TOTAL_SCORES = 5;
const int TOTAL_BGM = 3;
const int TOTAL_BG = 10;

// Tetromino identifiers
enum TetrominoFlag
{
  TETROMINO_I,
  TETROMINO_J,
  TETROMINO_L,
  TETROMINO_O,
  TETROMINO_S,
  TETROMINO_T,
  TETROMINO_Z,
  TETROMINO_NULL,
  TETROMINO_COUNT
};

// Square states
enum SquareState
{
  SQUARE_STATE_BLANK,
  SQUARE_STATE_MOVING,
  SQUARE_STATE_STILL,
  SQUARE_STATE_COUNT
};

// Square sprite types
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

// Game states
enum GameStateFlag
{
  GAME_STATE_NULL,
  GAME_STATE_INTRO,
  GAME_STATE_PLAY,
  GAME_STATE_GAMEOVER,
  GAME_STATE_SCORELIST,
  GAME_STATE_ERROR
};

// Mix Channels
enum MixChannels
{
  MIX_CHANNEL_MUSIC,
  MIX_CHANNEL_MOVE,
  MIX_CHANNEL_HOLD,
  MIX_CHANNEL_LAND,
  MIX_CHANNEL_CLEAR,
  MIX_CHANNEL_TETRIS,
  MIX_CHANNEL_GAMEOVER
};

#endif
