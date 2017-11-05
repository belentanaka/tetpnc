#include <stdlib.h>

#include "../constants.h"
#include "functions.h"

void randomPermutation( int* numbers, int len )
{
  for( int i = 0; i < len; i++ )
  {
    numbers[ i ] = i;
  }

  for( int i = 0; i < len - 1; i++ )
  {
    int j = rand() % len;
    int temp = numbers[ i ];
    numbers[ i ] = numbers[ j ];
    numbers[ j ] = temp;
  }
}

TetrominoFlag randomTetromino( TetrominoFlag previous )
{
  int prevTetromino;
  switch( prevTetromino )
  {
    case TETROMINO_I:
      prevTetromino = TETROMINO_I;
      break;

    case TETROMINO_J:
      prevTetromino = TETROMINO_J;
      break;

    case TETROMINO_L:
      prevTetromino = TETROMINO_L;
      break;

    case TETROMINO_O:
      prevTetromino = TETROMINO_O;
      break;

    case TETROMINO_S:
      prevTetromino = TETROMINO_S;
      break;

    case TETROMINO_T:
      prevTetromino = TETROMINO_T;
      break;

    case TETROMINO_Z:
      prevTetromino = TETROMINO_Z;
      break;
  }

  int newTetromino = rand() % TETROMINO_COUNT ;
  if( newTetromino == prevTetromino || newTetromino == TETROMINO_NULL )
  {
    newTetromino = rand() % TETROMINO_NULL;
  }

  switch( newTetromino )
  {
    case TETROMINO_I:
      return TETROMINO_I;

    case TETROMINO_J:
      return TETROMINO_J;

    case TETROMINO_L:
      return TETROMINO_L;

    case TETROMINO_O:
      return TETROMINO_O;

    case TETROMINO_S:
      return TETROMINO_S;

    case TETROMINO_T:
      return TETROMINO_T;

    case TETROMINO_Z:
      return TETROMINO_Z;
  }
}
